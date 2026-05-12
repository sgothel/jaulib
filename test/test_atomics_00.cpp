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

#include <thread>

#include <jau/test/catch2_ext.hpp>

#include <jau/debug.hpp>
#include <jau/ordered_atomic.hpp>

using namespace jau;

static thread_local int tls_counter = 0;

#define USE_LOGGING 0

/**
 * test_atomics_00: Tests a simple ping/pong based on atomics w/ spin-locks and atomic-wait from 2 threads
 * - plain atomics w/ spin-lock (busy loop) or atomic-wait
 * - using TLS
 * - no mutex lock
 * - no conditional signaling
 *
 * Validates basic atomic thread-safety and thread-local storage (TLS).
 */
template<bool USE_CPP20_ATOMIC_WAIT>
class TestAtomics00 {
  private:
    template <typename T>
    static void wait(const T &atom, typename T::value_type __old) noexcept {
        if constexpr (USE_CPP20_ATOMIC_WAIT) {
            atom.wait(__old); // atomic-wait, blocks until notified
        } else {
            // spin-lock waiting
            typename T::value_type __val = atom;
            while (__val == __old) {
                __val = atom;
            }
        }
    }

  #if USE_LOGGING == 0

    // no logging

    template <typename T>
    static typename T::value_type wait_for(const T &atom, typename T::value_type __new) noexcept {
        if constexpr (USE_CPP20_ATOMIC_WAIT) {
            return atom.wait_for(__new);
        } else {
            // spin-lock waiting
            const typename T::value_type __start = atom;
            typename T::value_type __old = __start;
            while (__new != __old) {
                __old = atom;
            }
            return __start;
        }
    }

    template <typename T>
    static void notify_one(T &atom) noexcept {
        if constexpr (USE_CPP20_ATOMIC_WAIT) {
            atom.notify_one();
        } else {
            // else spin-lock busy loop used
            (void) atom;
        }
    }

  #else

    // with logging

    template <typename T>
    static typename T::value_type wait_for(const T &atom, typename T::value_type __new) noexcept {
        if constexpr (USE_CPP20_ATOMIC_WAIT) {
            const typename T::value_type __start = atom;
            typename T::value_type __old = __start;
            while (__new != __old) {
                fprintf(stderr, "wait.W0: old %d != new %d\n", __old, __new);
                atom.wait(__old);
                __old = atom;
            }
            fprintf(stderr, "wait.WX: old %d == new %d\n", __old, __new);
            return __start;
        } else {
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
    }

    template <typename T>
    static void notify_one(T &atom) noexcept {
        if constexpr (USE_CPP20_ATOMIC_WAIT) {
            fprintf(stderr, "NNN.W: %d\n", atom.load());
            atom.notify_one();
        } else {
            // else spin-lock busy loop used
            fprintf(stderr, "NNN.S: %d\n", atom.load());
            (void) atom;
        }
    }

  #endif

    static constexpr const int PingCount = 50;
    static constexpr const int IDLE = 0;
    static constexpr const int PING = 1;
    static constexpr const int PONG = 2;

    jau::relaxed_atomic_int pingpong_counter = 0;
    jau::sc_atomic_int pingpong_status = IDLE;
    jau::sc_atomic_int threads_ready = 0; // ensure both threads have started, allowing atomic-wait being notified

    struct PingPong {
        int status = IDLE;
        int total_counter = 0;
        int ping_counter = 0;
        int pong_counter = 0;
    };
    jau::ordered_atomic<PingPong, std::memory_order_relaxed> pingpong;

    void reset(int status) {
        threads_ready = 0;
        PingPong pp;
        pp.status = status;
        pp.total_counter = 0;
        pp.ping_counter = 0;
        pp.pong_counter = 0;
        pingpong = pp;
        pingpong_counter = 0;
        pingpong_status = status;
        tls_counter = 0;
    }

    void ping() {
      int count = PingCount;
      ++threads_ready;
      jau_fprintf(stderr, "ping: START: %d\n", threads_ready.load());
      notify_one(threads_ready);

      while(count--) {
          wait_for(pingpong_status, PING);

          REQUIRE (pingpong_status == PING);
          PingPong pp = pingpong;
          REQUIRE (pp.status == PING);
          REQUIRE (pingpong_counter == pp.total_counter);
          REQUIRE (pp.total_counter == pp.ping_counter + pp.pong_counter);
          REQUIRE (tls_counter == pp.ping_counter);

          pingpong_counter = pingpong_counter + 1;
          ++tls_counter;
          ++pp.ping_counter;
          ++pp.total_counter;
          pp.status = PONG;

          #if USE_LOGGING
            jau_fprintf(stderr, "ping: ping_counter %d, left %d\n", pp.ping_counter, count);
          #endif

          pingpong = pp;
          pingpong_status = PONG;
          notify_one(pingpong_status);
      }
      jau_fprintf(stderr, "ping: END\n");
    }

    void pong() {
      int count = PingCount;
      ++threads_ready;
      jau_fprintf(stderr, "pong: START: %d\n", threads_ready.load());
      notify_one(threads_ready);

      while(count--) {
          wait_for(pingpong_status, PONG);

          REQUIRE (pingpong_status == PONG);
          PingPong pp = pingpong;
          REQUIRE (pp.status == PONG);
          REQUIRE (pingpong_counter == pp.total_counter);
          REQUIRE (pp.total_counter == pp.ping_counter + pp.pong_counter);
          REQUIRE (tls_counter == pp.pong_counter);

          pingpong_counter = pingpong_counter + 1;
          ++tls_counter;
          ++pp.pong_counter;
          ++pp.total_counter;
          pp.status = count ? PING : IDLE;

          #if USE_LOGGING
            jau_fprintf(stderr, "pong: pong_counter %d, left %d\n", pp.pong_counter, count);
          #endif

          pingpong = pp;
          pingpong_status = count ? PING : IDLE;
          notify_one(pingpong_status);
      }
      jau_fprintf(stderr, "pong: END\n");
    }

  public:
    void test01(bool startPongFirst) {
      jau_fprintf(stderr, "MAIN: p0 startPongFirst %s, is_lock_free (obj %s, type %s), USE_CPP20_ATOMIC_WAIT %s\n",
        startPongFirst, pingpong_status.is_lock_free(), std::atomic<int>::is_always_lock_free, USE_CPP20_ATOMIC_WAIT);

      reset(PING);
      std::thread threads[2];
      if (startPongFirst) {
          threads[0] = std::thread(&TestAtomics00::pong, this); // @suppress("Invalid arguments")
          threads[1] = std::thread(&TestAtomics00::ping, this); // @suppress("Invalid arguments")
      } else {
          threads[0] = std::thread(&TestAtomics00::ping, this); // @suppress("Invalid arguments")
          threads[1] = std::thread(&TestAtomics00::pong, this); // @suppress("Invalid arguments")
      }

      PingPong pp = pingpong;
      jau_fprintf(stderr, "MAIN: p1 threads %d, total: %d/%d, ping %d, pong %d\n",
        threads_ready.load(), pp.total_counter, PingCount, pp.ping_counter, pp.pong_counter);

      wait_for(threads_ready, 2); // ensure both threads have started, allowing atomic-wait being notified

      jau_fprintf(stderr, "MAIN: p2 threads %d, total: %d/%d, ping %d, pong %d\n",
        threads_ready.load(), pp.total_counter, PingCount, pp.ping_counter, pp.pong_counter);

      for (std::thread &t : threads) {
          if (t.joinable()) {
              t.join();
          }
      }
      pp = pingpong;
      jau_fprintf(stderr, "MAIN: p3 total: %d/%d, ping %d, pong %d\n", pp.total_counter, PingCount, pp.ping_counter, pp.pong_counter);
      REQUIRE (PingCount == pp.ping_counter);
      REQUIRE (PingCount == pp.pong_counter);
      REQUIRE (2*PingCount == pp.total_counter);
      REQUIRE (2*PingCount == pingpong_counter);
      REQUIRE (pp.status == IDLE);
      REQUIRE (pingpong_status == IDLE);

      REQUIRE (tls_counter == 0);


      jau_fprintf(stderr, "MAIN: XXX\n");
    }
};

static int loops = 4;

static void test_atomics_spin_lock() {
    bool startPongFirst = true;
    for(int i=loops; i>0; i--) {
        TestAtomics00<false> ta;
        ta.test01(startPongFirst);
        startPongFirst = !startPongFirst;
    }
}

#if __cplusplus > 201703L // C++20
    static void test_atomics_atomic_wait() {
        bool startPongFirst = true;
        for(int i=loops; i>0; i--) {
            TestAtomics00<true> ta;
            ta.test01(startPongFirst);
            startPongFirst = !startPongFirst;
        }
    }
#endif

METHOD_AS_TEST_CASE( test_atomics_spin_lock, "Test TestAtomics 00 - spin_lock");

#if __cplusplus > 201703L // C++20
    METHOD_AS_TEST_CASE( test_atomics_atomic_wait, "Test TestAtomics 00 - atomic_wait");
#endif
