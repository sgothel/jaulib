/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2022 Gothel Software e.K.
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
 */

#ifndef JAU_BACKTRACE_HPP_
#define JAU_BACKTRACE_HPP_

#include <string>

#include <jau/int_types.hpp>

namespace jau {

    /**
     * Returns a de-mangled backtrace string separated by newline excluding this function.
     * <p>
     * Returns one line per stack frame, each with
     * <pre>
     * - stack frame number
     * - demangled function name + offset (sp)
     * - the instruction pointer (pc)
     * - the stack pointer (sp)
     * </pre>
     * </p>
     * @param skip_anon_frames if true, skip all frames w/o proc-name
     * @param max_frames number of stack frames to be printed or -1 for unlimited, defaults to -1
     * @param skip_frames number of stack frames to skip, default is one frame for this function.
     * @return the de-mangled backtrace, separated by newline
     */
    std::string get_backtrace(const bool skip_anon_frames, const jau::snsize_t max_frames=-1, const jau::snsize_t skip_frames=1) noexcept;

    /**
     * Prints the de-mangled backtrace string separated by newline excluding this function to stderr, using get_backtrace().
     * @param skip_anon_frames if true, skip all frames w/o proc-name
     * @param max_frames number of stack frames to be printed or -1 for unlimited, defaults to -1
     * @param skip_frames number of stack frames to skip, default is two frames for this function and for get_backtrace().
     * @see get_backtrace()
     */
    void print_backtrace(const bool skip_anon_frames, const jau::snsize_t max_frames=-1, const jau::snsize_t skip_frames=2) noexcept;

} // namespace jau

#endif /* JAU_BACKTRACE_HPP_ */
