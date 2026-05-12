/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2026 Gothel Software e.K.
 *
 * SPDX-License-Identifier: MIT
 *
 * This Source Code Form is subject to the terms of the MIT License
 * If a copy of the MIT was not distributed with this file,
 * you can obtain one at https://opensource.org/license/mit/.
 */
#include <cassert>
#include <cstring>

#include <atomic>

#include <memory>
#include <thread>

#include <jau/test/catch2_ext.hpp>

#include <jau/debug.hpp>
#include <jau/ordered_atomic.hpp>

using namespace jau;

#define USE_LOGGING 0

/**
 * test_sharedptr_00: Tests std::shared_ptr thread-safe reference counter from n threads
 * - plain atomics w/ spin-lock (busy loop)
 * - plain std::shared_ptr ref-counting
 * - using TLS
 *
 * Validates basic std::shared_ptr thread-safety,
 * assuming atomic thread-safety and thread-local storage (TLS) is given, see `test_atomics_00.cpp`.
 */
class TestSharedPtr00 {
  private:
    template <typename T>
    static void wait(const T &atom, typename T::value_type __old) noexcept {
        // spin-lock waiting
        typename T::value_type __val = atom;
        while (__val == __old) {
            __val = atom;
        }
    }

  #if USE_LOGGING == 0

    // no logging

    template <typename T>
    static typename T::value_type wait_for(const T &atom, typename T::value_type __new) noexcept {
        // spin-lock waiting
        const typename T::value_type __start = atom;
        typename T::value_type __old = __start;
        while (__new != __old) {
            __old = atom;
        }
        return __start;
    }

    template <typename T>
    static void notify_all(T &atom) noexcept {
        // else spin-lock busy loop used
        (void) atom;
    }

  #else

    // with logging

    template <typename T>
    static typename T::value_type wait_for(const T &atom, typename T::value_type __new) noexcept {
        // spin-lock waiting
        const typename T::value_type __start = atom;
        typename T::value_type __old = __start;
        while (__new != __old) {
            fprintf(stderr, "wait.S0: old %d != new %d\n", __old, __new);
            __old = atom;
        }
        fprintf(stderr, "wait.SX: old %d == new %d\n", __old, __new);
        return __start;
    }

    template <typename T>
    static void notify_all(T &atom) noexcept {
        // else spin-lock busy loop used
        fprintf(stderr, "NNN.S: %d\n", atom.load());
        (void) atom;
    }

  #endif

    static constexpr const int ThreadCount = 10;
    static constexpr const int RefCpyPerThread = 10;

    jau::relaxed_atomic_int threads_work_done = 0;
    jau::sc_atomic_int threads_started = 0;
    jau::sc_atomic_int start = 0;
    jau::sc_atomic_int shutdown = 0;

    struct Blob {
        int the_universe = 42;
        jau::sc_atomic_int counter = 0;
    };
    typedef std::shared_ptr<Blob> SharedBlob;

    void reset() {
        threads_started = 0;
        start = 0;
        shutdown = 0;
        threads_work_done = 0;
    }

    void ref_thread(const SharedBlob *blob) {
      [[maybe_unused]] int t = ++threads_started;
      notify_all(threads_started);
      // jau_fprintf(stderr, "ref[t %d/?]: START (waiting)\n", t);

      [[maybe_unused]] int r = 0;
      wait_for(start, 1);
      {
          [[maybe_unused]] size_t r1 = blob->use_count();

          (*blob)->counter++;
          SharedBlob copies[RefCpyPerThread];
          for (SharedBlob &s : copies) {
            s = *blob;
          }
          [[maybe_unused]] size_t r3 = 0;
          for (SharedBlob &s : copies) {
            s->counter++;
            r3 += s.use_count();
          }

          r = ++threads_work_done;
          [[maybe_unused]] size_t r2 = blob->use_count();
          // jau_fprintf(stderr, "ref[t %d/%d]: %zu -> %zu/%zu\n", t, r, r1, r2, r3);
          notify_all(threads_work_done);

          wait_for(shutdown, 1);
      }
      --threads_started;
      notify_all(threads_started);

      // jau_fprintf(stderr, "ref[t %d/%d]: END\n", t, r);
    }

  public:
    void test01() {
      SharedBlob genesis = std::make_shared<Blob>();

      jau_fprintf(stderr, "MAIN: p0 is_lock_free (obj %s, type %s, ref[c %zu])\n",
        threads_work_done.is_lock_free(), std::atomic<int>::is_always_lock_free, genesis.use_count());

      reset();
      std::thread threads[ThreadCount];
      for (std::thread &t : threads) {
          t = std::thread(&TestSharedPtr00::ref_thread, this, &genesis); // @suppress("Invalid arguments")
      }

      wait_for(threads_started, ThreadCount);
      jau_fprintf(stderr, "MAIN: p2 threads %d/%d, ref[c %zu], bc %d\n",
        threads_started.load(), threads_work_done.load(), genesis.use_count(), genesis->counter.load());

      start = 1;
      notify_all(start);
      wait_for(threads_work_done, ThreadCount);
      jau_fprintf(stderr, "MAIN: p3 threads %d/%d, ref[c %zu], bc %d\n",
        threads_started.load(), threads_work_done.load(), genesis.use_count(), genesis->counter.load());
      REQUIRE(threads_started == ThreadCount);
      REQUIRE(threads_work_done == ThreadCount);
      REQUIRE(genesis.use_count() == RefCpyPerThread*ThreadCount+1);
      REQUIRE(genesis->counter == (RefCpyPerThread+1)*ThreadCount);

      shutdown = 1;
      notify_all(shutdown);
      wait_for(threads_started, 0);
      jau_fprintf(stderr, "MAIN: p4 threads %d/%d, ref[c %zu], bc %d\n",
        threads_started.load(), threads_work_done.load(), genesis.use_count(), genesis->counter.load());
      REQUIRE(threads_started == 0);
      REQUIRE(threads_work_done == ThreadCount);
      REQUIRE(genesis.use_count() == 1);
      REQUIRE(genesis->counter == (RefCpyPerThread+1)*ThreadCount);

      for (std::thread &t : threads) {
          if (t.joinable()) {
              t.join();
          }
      }

      jau_fprintf(stderr, "MAIN: XXX\n");
    }
};

static int loops = 4;

static void test_sharedptr01() {
    bool startPongFirst = true;
    for(int i=loops; i>0; i--) {
        TestSharedPtr00 ta;
        ta.test01();
        startPongFirst = !startPongFirst;
    }
}

METHOD_AS_TEST_CASE( test_sharedptr01, "Test TestSharedPtr 00");
