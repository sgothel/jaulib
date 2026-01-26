/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2020 Gothel Software e.K.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *************************************************************************
 *
 * Distributed under the Boost Software License, Version 1.0.
 *  (See accompanying file LICENSE_1_0.txt or copy at
 *        https://www.boost.org/LICENSE_1_0.txt)
 *
 * SPDX-License-Identifier: BSL-1.0
 */

#ifndef CATCH2_MY_MAIN_H
#define CATCH2_MY_MAIN_H

#include <vector>

#if defined(__EMSCRIPTEN__)
extern "C" {
    #include <unistd.h>
    #include <sys/random.h>
    #include <errno.h>

    ssize_t getrandom(void* buffer, size_t len, unsigned int flags) {
        (void)flags;
        char *pos = (char*)buffer;
        for(size_t i=0; i<len; ++i) {
            *pos = (char)(i%255);
            pos++;
        }
        return len;
    }
    int getentropy(void *buffer, size_t len)
    {
        // int cs;
        int ret = 0;
        char *pos = (char*)buffer;

        if (len > 256) {
            errno = EIO;
            return -1;
        }

        // pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &cs);

        while (len) {
            ret = getrandom(pos, len, 0);
            if (ret < 0) {
                if (errno == EINTR) continue;
                else break;
            }
            pos += ret;
            len -= ret;
            ret = 0;
        }

        // pthread_setcancelstate(cs, 0);

        return ret;
    }
}
#endif // __EMSCRIPTEN__

#define CATCH_AMALGAMATED_CUSTOM_MAIN 1
#include <catch2/catch_amalgamated.hpp>

/** The main argv[0] test executable path */
std::string executable_path; // NOLINT(misc-use-internal-linkage)

/** Run w/o command-line args, i.e. default CI unit test. */
bool catch_auto_run; // NOLINT(misc-use-internal-linkage)

/** Run w/ command-line arg '--perf_analysis'. */
bool catch_perf_analysis; // NOLINT(misc-use-internal-linkage)

static char * extra_args[] = { (char*)"--colour-mode", (char*)"none" };
static int extra_args_c = sizeof(extra_args) / sizeof(const char *);

int main( int argc, char* argv[] )
{
  Catch::Session session; // There must be exactly one instance

  if( 1 <= argc ) {
      executable_path = std::string(argv[0]);
  } else {
      executable_path = "undef";
  }

  catch_auto_run = ( 1 >= argc );

  std::vector<char*> argv_2;
  argv_2.reserve(argc+extra_args_c);

  for(int i=0; i<argc; i++) {
      if( 0 == strcmp("--perf-analysis", argv[i]) ) {
          catch_perf_analysis = true;
      } else {
          argv_2.push_back(argv[i]);
      }
  }
  for(int i=0; i<extra_args_c; i++) {
      argv_2.push_back(extra_args[i]);
  }
  fprintf(stderr, "argc %d -> %zu, auto_run %d, perf-analysis %d\n",
          argc, argv_2.size(), catch_auto_run, catch_perf_analysis);
  for(size_t i=0; i<argv_2.size(); ++i) {
      printf("[%zu] %s\n", i, argv_2[i]);
  }

  // writing to session.configData() here sets defaults
  // this is the preferred way to set them

  int returnCode = session.applyCommandLine( int(argv_2.size()), argv_2.data() );

  if( returnCode != 0 ) { // Indicates a command line error
      return returnCode;
  }

  if( catch_auto_run ) {
      const char* my_args[] = { argv[0],
                                "--benchmark-warmup-time", "1",
                                "--benchmark-confidence-interval", "0",
                                "--benchmark-samples", "1",
                                "--benchmark-resamples", "0",
                                "--benchmark-no-analysis"
                              };
      int res = session.applyCommandLine( sizeof(my_args)/sizeof(char*), my_args );
      if( 0 != res ) {
          return res;
      }
      // Catch::Config& config = session.config();
  }

  // writing to session.configData() or session.Config() here
  // overrides command line args
  // only do this if you know you need to

  int numFailed = session.run();

  // numFailed is clamped to 255 as some unices only use the lower 8 bits.
  // This clamping has already been applied, so just return it here
  // You can also do any post run clean-up here
  return numFailed;
}

#endif // CATCH2_MY_MAIN_H

