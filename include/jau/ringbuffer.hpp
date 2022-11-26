/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2020 Gothel Software e.K.
 * Copyright (c) 2020 ZAFENA AB
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

#ifndef JAU_RINGBUFFER_HPP_
#define JAU_RINGBUFFER_HPP_

#include <type_traits>
#include <atomic>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <algorithm>

#include <cstring>
#include <string>
#include <cstdint>

#include <jau/debug.hpp>
#include <jau/basic_types.hpp>
#include <jau/ordered_atomic.hpp>
#include <jau/fraction_type.hpp>
#include <jau/callocator.hpp>

namespace jau {

#if 0
    #define _DEBUG_DUMP(...) { dump(stderr, __VA_ARGS__); }
    #define _DEBUG_DUMP2(a, ...) { a.dump(stderr, __VA_ARGS__); }
    #define _DEBUG_PRINT(...) { fprintf(stderr, __VA_ARGS__); }
#else
    #define _DEBUG_DUMP(...)
    #define _DEBUG_DUMP2(a, ...)
    #define _DEBUG_PRINT(...)
#endif

/** @defgroup DataStructs Data Structures
 * Data structures, notably
 *  - \ref ringbuffer
 *  - \ref darray
 *  - cow_darray
 *  - cow_vector
 *
 *  @{
 */

/**
 * Ring buffer implementation, a.k.a circular buffer,
 * exposing <i>lock-free</i>
 * {@link #get() get*(..)} and {@link #put(Object) put*(..)} methods.
 *
 * This data structure is also supporting \ref Concurrency.
 *
 * Implementation utilizes the <i>Always Keep One Slot Open</i>,
 * hence implementation maintains an internal array of `capacity` <i>plus one</i>!
 *
 * ### Characteristics
 * - Read position points to the last read element.
 * - Write position points to the last written element.
 *
 * <table border="1">
 *   <tr><td>Empty</td><td>writePos == readPos</td><td>size == 0</td></tr>
 *   <tr><td>Full</td><td>writePos == readPos - 1</td><td>size == capacity</td></tr>
 * </table>
 * <pre>
 * Empty [RW][][ ][ ][ ][ ][ ][ ] ; W==R
 * Avail [ ][ ][R][.][.][.][.][W] ; W > R
 * Avail [.][.][.][W][ ][ ][R][.] ; W <  R - 1
 * Full  [.][.][.][.][.][W][R][.] ; W==R-1
 * </pre>
 *
 *
 * ### Thread Safety
 * Thread safety is guaranteed, considering the mode of operation as described below.
 *
 * @anchor ringbuffer_single_pc
 * #### One producer-thread and one consumer-thread
 * Expects one producer-thread at a time and one consumer-thread at a time concurrently.
 * Threads can be different or the same.
 *
 * This is the default mode with the least std::mutex operations.
 * - Only blocking producer put() and consumer get() waiting for
 * free slots or available data will utilize lock and wait for the corresponding operation.
 * - Otherwise implementation is <i>lock-free</i> and relies on SC-DRF via atomic memory barriers.
 *
 * See setMultiPCEnabled().
 *
 * Implementation is thread safe if:
 * - {@link #put() put*(..)} operations from one producer-thread at a time.
 * - {@link #get() get*(..)} operations from one consumer-thread at a time.
 * - {@link #put() put*(..)} producer and {@link #get() get*(..)} consumer threads can be different or the same.
 *
 * @anchor ringbuffer_multi_pc
 * #### Multiple producer-threads and multiple consumer-threads
 * Expects multiple producer-threads and multiple consumer-threads concurrently.
 * Threads can be different or the same.
 *
 * This operation mode utilizes a specific multi-producer and -consumer lock,
 * synchronizing {@link #put() put*(..)} and {@link #get() get*(..)} operations separately.
 *
 * Use setMultiPCEnabled() to enable or disable multiple producer and consumer mode.
 *
 * Implementation is thread safe if:
 * - {@link #put() put*(..)} operations concurrently from multiple threads.
 * - {@link #get() get*(..)} operations concurrently from multiple threads.
 * - {@link #put() put*(..)} producer and {@link #get() get*(..)} consumer threads can be different or the same.
 *
 * #### See also
 * - Sequentially Consistent (SC) ordering or SC-DRF (data race free) <https://en.cppreference.com/w/cpp/atomic/memory_order#Sequentially-consistent_ordering>
 * - std::memory_order <https://en.cppreference.com/w/cpp/atomic/memory_order>
 * - jau::sc_atomic_critical
 * - setMultiPCEnabled()
 *
 * @anchor ringbuffer_ntt_params
 * ### Non-Type Template Parameter (NTTP) controlling Value_type memory
 * See @ref darray_ntt_params.
 *
 * #### use_memmove
 * `use_memmove` see @ref darray_memmove.
 *
 * #### use_memcpy
 * `use_memcpy` has more strict requirements than `use_memmove`,
 * i.e. strictly relies on Value_type being `std::is_trivially_copyable_v<Value_type>`.
 *
 * It allows to merely use memory operations w/o the need for constructor or destructor.
 *
 * See [Trivial destructor](https://en.cppreference.com/w/cpp/language/destructor#Trivial_destructor)
 * being key requirement to [TriviallyCopyable](https://en.cppreference.com/w/cpp/named_req/TriviallyCopyable).
 * > A trivial destructor is a destructor that performs no action.
 * > Objects with trivial destructors don't require a delete-expression and may be disposed of by simply deallocating their storage.
 * > All data types compatible with the C language (POD types) are trivially destructible.`
 *
 * #### use_secmem
 * `use_secmem` see @ref darray_secmem.
 *
 * @see @ref darray_ntt_params
 * @see jau::sc_atomic_critical
 */
template <typename Value_type, typename Size_type,
          bool use_memmove = std::is_trivially_copyable_v<Value_type> || is_container_memmove_compliant_v<Value_type>,
          bool use_memcpy  = std::is_trivially_copyable_v<Value_type>,
          bool use_secmem  = is_enforcing_secmem_v<Value_type>
         >
class ringbuffer {
    public:
        constexpr static const bool uses_memmove = use_memmove;
        constexpr static const bool uses_memcpy = use_memcpy;
        constexpr static const bool uses_secmem  = use_secmem;

        // typedefs' for C++ named requirements: Container (ex iterator)

        typedef Value_type                                  value_type;
        typedef value_type*                                 pointer;
        typedef const value_type*                           const_pointer;
        typedef value_type&                                 reference;
        typedef const value_type&                           const_reference;
        typedef Size_type                                   size_type;
        typedef typename std::make_signed<size_type>::type  difference_type;

        typedef jau::callocator<Value_type>                 allocator_type;

    private:
        constexpr static const bool is_integral = std::is_integral_v<Value_type>;

        typedef std::remove_const_t<Value_type>             value_type_mutable;
        /** Required to create and move immutable elements, aka const */
        typedef value_type_mutable*                         pointer_mutable;

        static constexpr void* voidptr_cast(const_pointer p) { return reinterpret_cast<void*>( const_cast<pointer_mutable>( p ) ); }

        /** SC atomic integral scalar jau::nsize_t. Memory-Model (MM) guaranteed sequential consistency (SC) between acquire (read) and release (write) */
        typedef ordered_atomic<Size_type, std::memory_order::memory_order_seq_cst> sc_atomic_Size_type;

        /** Relaxed non-SC atomic integral scalar jau::nsize_t. Memory-Model (MM) only guarantees the atomic value, _no_ sequential consistency (SC) between acquire (read) and release (write). */
        typedef ordered_atomic<Size_type, std::memory_order::memory_order_relaxed> relaxed_atomic_Size_type;

        /**
         * Flagging whether multiple-producer and -consumer are enabled,
         * see @ref ringbuffer_multi_pc and @ref ringbuffer_single_pc.
         *
         * Defaults to `false`.
         */
        bool multi_pc_enabled = false;

        /** synchronizes write-operations (put*), i.e. modifying the writePos. */
        mutable std::mutex syncWrite, syncMultiWrite; // Memory-Model (MM) guaranteed sequential consistency (SC) between acquire and release
        std::condition_variable cvWrite;

        /** synchronizes read-operations (get*), i.e. modifying the readPos. */
        mutable std::mutex syncRead,  syncMultiRead;  // Memory-Model (MM) guaranteed sequential consistency (SC) between acquire and release
        std::condition_variable cvRead;

        jau::relaxed_atomic_bool interrupted_read = false;
        jau::relaxed_atomic_bool interrupted_write = false;

        allocator_type alloc_inst;

        /* const */ Size_type capacityPlusOne;  // not final due to grow
        /* const */ Value_type * array;         // Synchronized due to MM's data-race-free SC (SC-DRF) between [atomic] acquire/release
        sc_atomic_Size_type readPos;     // Memory-Model (MM) guaranteed sequential consistency (SC) between acquire (read) and release (write)
        sc_atomic_Size_type writePos;    // ditto

        constexpr Value_type * newArray(const Size_type count) noexcept {
            if( 0 < count ) {
                value_type * m = alloc_inst.allocate(count);
                if( nullptr == m ) {
                    // Avoid exception, abort!
                    ABORT("Error: bad_alloc: alloc %zu  elements * %zu bytes/element = %zu  bytes failed",
                          count, sizeof(value_type), (count * sizeof(value_type)));
                }
                _DEBUG_DUMP("newArray ...");
                _DEBUG_PRINT("newArray %" PRIu64 "\n", count);
                return m;
            } else {
                _DEBUG_DUMP("newArray ...");
                _DEBUG_PRINT("newArray %" PRIu64 "\n", count);
                return nullptr;
            }
        }

        constexpr void freeArray(Value_type ** a, const Size_type count) noexcept {
            _DEBUG_DUMP("freeArray(def)");
            _DEBUG_PRINT("freeArray %p\n", *a);
            if( nullptr != *a ) {
                alloc_inst.deallocate(*a, count);
                *a = nullptr;
            } else {
                ABORT("ringbuffer::freeArray with nullptr");
            }
        }

        constexpr void dtor_one(const Size_type pos) {
            ( array + pos )->~value_type(); // placement new -> manual destruction!
            if constexpr ( uses_secmem ) {
                ::explicit_bzero(voidptr_cast(array + pos), sizeof(value_type));
            }
        }
        constexpr void dtor_one(pointer elem) {
            ( elem )->~value_type(); // placement new -> manual destruction!
            if constexpr ( uses_secmem ) {
                ::explicit_bzero(voidptr_cast(elem), sizeof(value_type));
            }
        }

        Size_type waitForElementsImpl(const Size_type min_count, const fraction_i64& timeout) noexcept {
            Size_type available = size();
            if( min_count > available && min_count < capacityPlusOne ) {
                interrupted_read = false;
                std::unique_lock<std::mutex> lockWrite(syncWrite); // SC-DRF w/ putImpl via same lock
                available = size();
                const fraction_timespec timeout_time = getMonotonicTime() + fraction_timespec(timeout);
                while( !interrupted_read && min_count > available ) {
                    if( fractions_i64::zero == timeout ) {
                        cvWrite.wait(lockWrite);
                        available = size();
                    } else {
                        std::cv_status s = wait_until(cvWrite, lockWrite, timeout_time );
                        available = size();
                        if( std::cv_status::timeout == s && min_count > available ) {
                            return available;
                        }
                    }
                }
                if( interrupted_read ) {
                    interrupted_read = false;
                }
            }
            return available;
        }

        Size_type waitForFreeSlotsImpl(const Size_type min_count, const fraction_i64& timeout) noexcept {
            Size_type available = freeSlots();
            if( min_count > available && min_count < capacityPlusOne ) {
                interrupted_write = false;
                std::unique_lock<std::mutex> lockRead(syncRead); // SC-DRF w/ getImpl via same lock
                available = freeSlots();
                const fraction_timespec timeout_time = getMonotonicTime() + fraction_timespec(timeout);
                while( !interrupted_write && min_count > available ) {
                    if( fractions_i64::zero == timeout ) {
                        cvRead.wait(lockRead);
                        available = freeSlots();
                    } else {
                        std::cv_status s = wait_until(cvRead, lockRead, timeout_time );
                        available = freeSlots();
                        if( std::cv_status::timeout == s && min_count > available ) {
                            return available;
                        }
                    }
                }
                if( interrupted_write ) {
                    interrupted_write = false;
                }
            }
            return available;
        }

        /**
         * clear all elements, zero size.
         *
         * Moves readPos == writePos compatible with put*() when waiting for available
         */
        constexpr void clearImpl() noexcept {
            const Size_type size_ = size();
            if( 0 < size_ ) {
                if constexpr ( use_memcpy ) {
                    if constexpr ( uses_secmem ) {
                        ::explicit_bzero(voidptr_cast(&array[0]), capacityPlusOne*sizeof(Value_type));
                    }
                    readPos = writePos.load();
                } else {
                    Size_type localReadPos = readPos;
                    for(Size_type i=0; i<size_; i++) {
                        localReadPos = (localReadPos + 1) % capacityPlusOne;
                        ( array + localReadPos )->~value_type(); // placement new -> manual destruction!
                    }
                    if( writePos != localReadPos ) {
                        // Avoid exception, abort!
                        ABORT("copy segment error: this %s, readPos %d/%d; writePos %d", toString().c_str(), readPos.load(), localReadPos, writePos.load());
                    }
                    if constexpr ( uses_secmem ) {
                        ::explicit_bzero(voidptr_cast(&array[0]), capacityPlusOne*sizeof(Value_type));
                    }
                    readPos = localReadPos;
                }
            }
        }

        constexpr void clearAndZeroMemImpl() noexcept {
            if constexpr ( use_memcpy ) {
                ::explicit_bzero(voidptr_cast(&array[0]), capacityPlusOne*sizeof(Value_type));
                readPos = writePos.load();
            } else {
                const Size_type size_ = size();
                Size_type localReadPos = readPos;
                for(Size_type i=0; i<size_; i++) {
                    localReadPos = (localReadPos + 1) % capacityPlusOne;
                    ( array + localReadPos )->~value_type(); // placement new -> manual destruction!
                }
                if( writePos != localReadPos ) {
                    // Avoid exception, abort!
                    ABORT("copy segment error: this %s, readPos %d/%d; writePos %d", toString().c_str(), readPos.load(), localReadPos, writePos.load());
                }
                ::explicit_bzero(voidptr_cast(&array[0]), capacityPlusOne*sizeof(Value_type));
                readPos = localReadPos;
            }
        }

        void cloneFrom(const bool allocArrayAndCapacity, const ringbuffer & source) noexcept {
            if( allocArrayAndCapacity ) {
                if( nullptr != array ) {
                    clearImpl();
                    freeArray(&array, capacityPlusOne);
                }
                capacityPlusOne = source.capacityPlusOne;
                array = newArray(capacityPlusOne);
            } else if( capacityPlusOne != source.capacityPlusOne ) {
                ABORT( ("capacityPlusOne not equal: this "+toString()+", source "+source.toString() ).c_str() );
            } else {
                clearImpl();
            }

            readPos = source.readPos.load();
            writePos = source.writePos.load();

            if constexpr ( uses_memcpy ) {
                ::memcpy(voidptr_cast(&array[0]),
                         &source.array[0],
                         capacityPlusOne*sizeof(Value_type));
            } else {
                const Size_type size_ = size();
                Size_type localWritePos = readPos;
                for(Size_type i=0; i<size_; i++) {
                    localWritePos = (localWritePos + 1) % capacityPlusOne;
                    new (const_cast<pointer_mutable>(array + localWritePos)) value_type( source.array[localWritePos] ); // placement new
                }
                if( writePos != localWritePos ) {
                    ABORT( ("copy segment error: this "+toString()+", localWritePos "+std::to_string(localWritePos)+"; source "+source.toString()).c_str() );
                }
            }
        }

        ringbuffer& assignCopyImpl(const ringbuffer &_source) noexcept {
            if( this == &_source ) {
                return *this;
            }

            if( capacityPlusOne != _source.capacityPlusOne ) {
                cloneFrom(true, _source);
            } else {
                cloneFrom(false, _source);
            }
            _DEBUG_DUMP("assignment(copy.this)");
            _DEBUG_DUMP2(_source, "assignment(copy.source)");
            return *this;
        }

        void resetImpl(const Value_type * copyFrom, const Size_type copyFromCount) noexcept {
            // fill with copyFrom elements
            if( nullptr != copyFrom && 0 < copyFromCount ) {
                if( copyFromCount > capacityPlusOne-1 ) {
                    // new blank resized array
                    if( nullptr != array ) {
                        clearImpl();
                        freeArray(&array, capacityPlusOne);
                    }
                    capacityPlusOne = copyFromCount + 1;
                    array = newArray(capacityPlusOne);
                    readPos  = 0;
                    writePos = 0;
                } else {
                    clearImpl();
                }
                if constexpr ( uses_memcpy ) {
                    ::memcpy(voidptr_cast(&array[0]),
                             copyFrom,
                             copyFromCount*sizeof(Value_type));
                    readPos  = capacityPlusOne - 1; // last read-pos
                    writePos = copyFromCount   - 1; // last write-pos
                } else {
                    Size_type localWritePos = writePos;
                    for(Size_type i=0; i<copyFromCount; i++) {
                        localWritePos = (localWritePos + 1) % capacityPlusOne;
                        new (const_cast<pointer_mutable>(array + localWritePos)) value_type( copyFrom[i] ); // placement new
                    }
                    writePos = localWritePos;
                }
            } else {
                clearImpl();
            }
        }

        bool peekImpl(Value_type& dest, const bool blocking, const fraction_i64& timeout) noexcept {
            if( !std::is_copy_constructible_v<Value_type> ) {
                ABORT("Value_type is not copy constructible");
                return false;
            }
            if( 1 >= capacityPlusOne ) {
                return false;
            }
            const Size_type oldReadPos = readPos; // SC-DRF acquire atomic readPos, sync'ing with putImpl
            Size_type localReadPos = oldReadPos;
            if( localReadPos == writePos ) {
                if( blocking ) {
                    interrupted_read = false;
                    std::unique_lock<std::mutex> lockWrite(syncWrite); // SC-DRF w/ putImpl via same lock
                    const fraction_timespec timeout_time = getMonotonicTime() + fraction_timespec(timeout);
                    while( !interrupted_read && localReadPos == writePos ) {
                        if( fractions_i64::zero == timeout ) {
                            cvWrite.wait(lockWrite);
                        } else {
                            std::cv_status s = wait_until(cvWrite, lockWrite, timeout_time );
                            if( std::cv_status::timeout == s && localReadPos == writePos ) {
                                return false;
                            }
                        }
                    }
                    if( interrupted_read ) {
                        interrupted_read = false;
                        return false;
                    }
                } else {
                    return false;
                }
            }
            localReadPos = (localReadPos + 1) % capacityPlusOne;
            if constexpr ( !is_integral && uses_memmove ) {
                // must not dtor after memcpy; memcpy OK, not overlapping
                ::memcpy(voidptr_cast(&dest),
                         &array[localReadPos],
                         sizeof(Value_type));
            } else {
                dest = array[localReadPos];
            }
            readPos = oldReadPos; // SC-DRF release atomic readPos (complete acquire-release even @ peek)
            return true;
        }

        bool moveOutImpl(Value_type& dest, const bool blocking, const fraction_i64& timeout) noexcept {
            if( 1 >= capacityPlusOne ) {
                return false;
            }
            const Size_type oldReadPos = readPos; // SC-DRF acquire atomic readPos, sync'ing with putImpl
            Size_type localReadPos = oldReadPos;
            if( localReadPos == writePos ) {
                if( blocking ) {
                    interrupted_read = false;
                    std::unique_lock<std::mutex> lockWrite(syncWrite); // SC-DRF w/ putImpl via same lock
                    const fraction_timespec timeout_time = getMonotonicTime() + fraction_timespec(timeout);
                    while( !interrupted_read && localReadPos == writePos ) {
                        if( fractions_i64::zero == timeout ) {
                            cvWrite.wait(lockWrite);
                        } else {
                            std::cv_status s = wait_until(cvWrite, lockWrite, timeout_time );
                            if( std::cv_status::timeout == s && localReadPos == writePos ) {
                                return false;
                            }
                        }
                    }
                    if( interrupted_read ) {
                        interrupted_read = false;
                        return false;
                    }
                } else {
                    return false;
                }
            }
            localReadPos = (localReadPos + 1) % capacityPlusOne;
            if constexpr ( is_integral ) {
                dest = array[localReadPos];
                if constexpr ( uses_secmem ) {
                    ::explicit_bzero(voidptr_cast(&array[localReadPos]), sizeof(Value_type));
                }
            } else if constexpr ( uses_memmove ) {
                // must not dtor after memcpy; memcpy OK, not overlapping
                ::memcpy(voidptr_cast(&dest),
                         &array[localReadPos],
                         sizeof(Value_type));
                if constexpr ( uses_secmem ) {
                    ::explicit_bzero(voidptr_cast(&array[localReadPos]), sizeof(Value_type));
                }
            } else {
                dest = std::move( array[localReadPos] );
                dtor_one( localReadPos );
            }
            {
                std::unique_lock<std::mutex> lockRead(syncRead); // SC-DRF w/ putImpl via same lock
                readPos = localReadPos; // SC-DRF release atomic readPos
            }
            cvRead.notify_all(); // notify waiting putter, have mutex unlocked before notify_all to avoid pessimistic re-block of notified wait() thread.
            return true;
        }

        Size_type moveOutImpl(Value_type *dest, const Size_type dest_len, const Size_type min_count_, const bool blocking, const fraction_i64& timeout) noexcept {
            const Size_type min_count = std::min(dest_len, min_count_);
            Value_type *iter_out = dest;

            if( min_count >= capacityPlusOne ) {
                return 0;
            }
            if( 0 == min_count ) {
                return 0;
            }

            const Size_type oldReadPos = readPos; // SC-DRF acquire atomic readPos, sync'ing with putImpl
            Size_type localReadPos = oldReadPos;
            Size_type available = size();
            if( min_count > available ) {
                if( blocking ) {
                    interrupted_read = false;
                    std::unique_lock<std::mutex> lockWrite(syncWrite); // SC-DRF w/ putImpl via same lock
                    available = size();
                    const fraction_timespec timeout_time = getMonotonicTime() + fraction_timespec(timeout);
                    while( !interrupted_read && min_count > available ) {
                        if( fractions_i64::zero == timeout ) {
                            cvWrite.wait(lockWrite);
                            available = size();
                        } else {
                            std::cv_status s = wait_until(cvWrite, lockWrite, timeout_time );
                            available = size();
                            if( std::cv_status::timeout == s && min_count > available ) {
                                return 0;
                            }
                        }
                    }
                    if( interrupted_read ) {
                        interrupted_read = false;
                        return 0;
                    }
                } else {
                    return 0;
                }
            }
            const Size_type count = std::min(dest_len, available);

            /**
             * Empty [RW][][ ][ ][ ][ ][ ][ ][ ][ ][ ][ ][ ][ ][ ] ; W==R
             * Avail [ ][ ][R][.][.][.][.][W][ ][ ][ ][ ][ ][ ][ ] ; W > R
             * Avail [.][.][.][W][ ][ ][R][.][.][.][.][.][.][.][.] ; W <  R - 1
             * Full  [.][.][.][.][.][W][R][.][.][.][.][.][.][.][.] ; W==R-1
             */
            // Since available > 0, we can exclude Empty case.
            Size_type togo_count = count;
            const Size_type localWritePos = writePos;
            if( localReadPos > localWritePos ) {
                // we have a tail
                localReadPos = ( localReadPos + 1 ) % capacityPlusOne; // next-read-pos
                const Size_type tail_count = std::min(togo_count, capacityPlusOne - localReadPos);
                if constexpr ( uses_memmove ) {
                    // must not dtor after memcpy; memcpy OK, not overlapping
                    ::memcpy(voidptr_cast(iter_out),
                             &array[localReadPos],
                             tail_count*sizeof(Value_type));
                    if constexpr ( uses_secmem ) {
                        ::explicit_bzero(voidptr_cast(&array[localReadPos]), tail_count*sizeof(Value_type));
                    }
                } else {
                    for(Size_type i=0; i<tail_count; i++) {
                        iter_out[i] = std::move( array[localReadPos+i] );
                        dtor_one( localReadPos + i ); // manual destruction, even after std::move (object still exists)
                    }
                }
                localReadPos = ( localReadPos + tail_count - 1 ) % capacityPlusOne; // last read-pos
                togo_count -= tail_count;
                iter_out += tail_count;
            }
            if( togo_count > 0 ) {
                // we have a head
                localReadPos = ( localReadPos + 1 ) % capacityPlusOne; // next-read-pos
                if constexpr ( uses_memmove ) {
                    // must not dtor after memcpy; memcpy OK, not overlapping
                    ::memcpy(voidptr_cast(iter_out),
                             &array[localReadPos],
                             togo_count*sizeof(Value_type));
                    if constexpr ( uses_secmem ) {
                        ::explicit_bzero(voidptr_cast(&array[localReadPos]), togo_count*sizeof(Value_type));
                    }
                } else {
                    for(Size_type i=0; i<togo_count; i++) {
                        iter_out[i] = std::move( array[localReadPos+i] );
                        dtor_one( localReadPos + i ); // manual destruction, even after std::move (object still exists)
                    }
                }
                localReadPos = ( localReadPos + togo_count - 1 ) % capacityPlusOne; // last read-pos
            }
            {
                std::unique_lock<std::mutex> lockRead(syncRead); // SC-DRF w/ putImpl via same lock
                readPos = localReadPos; // SC-DRF release atomic readPos
            }
            cvRead.notify_all(); // notify waiting putter, have mutex unlocked before notify_all to avoid pessimistic re-block of notified wait() thread.
            return count;
        }

        Size_type dropImpl (Size_type count, const bool blocking, const fraction_i64& timeout) noexcept {
            if( count >= capacityPlusOne ) {
                if( blocking ) {
                    return 0;
                }
                count = capacityPlusOne-1; // claim theoretical maximum for non-blocking
            }
            if( 0 == count ) {
                return 0;
            }

            const Size_type oldReadPos = readPos; // SC-DRF acquire atomic readPos, sync'ing with putImpl
            Size_type localReadPos = oldReadPos;
            Size_type available = size();
            if( count > available ) {
                if( blocking ) {
                    interrupted_read = false;
                    std::unique_lock<std::mutex> lockWrite(syncWrite); // SC-DRF w/ putImpl via same lock
                    available = size();
                    const fraction_timespec timeout_time = getMonotonicTime() + fraction_timespec(timeout);
                    while( !interrupted_read && count > available ) {
                        if( fractions_i64::zero == timeout ) {
                            cvWrite.wait(lockWrite);
                            available = size();
                        } else {
                            std::cv_status s = wait_until(cvWrite, lockWrite, timeout_time );
                            available = size();
                            if( std::cv_status::timeout == s && count > available ) {
                                return 0;
                            }
                        }
                    }
                    if( interrupted_read ) {
                        interrupted_read = false;
                        return 0;
                    }
                } else {
                    count = available; // drop all available for non-blocking
                }
            }
            /**
             * Empty [RW][][ ][ ][ ][ ][ ][ ][ ][ ][ ][ ][ ][ ][ ] ; W==R
             * Avail [ ][ ][R][.][.][.][.][W][ ][ ][ ][ ][ ][ ][ ] ; W > R
             * Avail [.][.][.][W][ ][ ][R][.][.][.][.][.][.][.][.] ; W <  R - 1
             * Full  [.][.][.][.][.][W][R][.][.][.][.][.][.][.][.] ; W==R-1
             */
            // Since available > 0, we can exclude Empty case.
            Size_type togo_count = count;
            const Size_type localWritePos = writePos;
            if( localReadPos > localWritePos ) {
                // we have a tail
                localReadPos = ( localReadPos + 1 ) % capacityPlusOne; // next-read-pos
                const Size_type tail_count = std::min(togo_count, capacityPlusOne - localReadPos);
                if constexpr ( uses_memcpy ) {
                    if constexpr ( uses_secmem ) {
                        ::explicit_bzero(voidptr_cast(&array[localReadPos]), tail_count*sizeof(Value_type));
                    }
                } else {
                    for(Size_type i=0; i<tail_count; i++) {
                        dtor_one( localReadPos+i );
                    }
                }
                localReadPos = ( localReadPos + tail_count - 1 ) % capacityPlusOne; // last read-pos
                togo_count -= tail_count;
            }
            if( togo_count > 0 ) {
                // we have a head
                localReadPos = ( localReadPos + 1 ) % capacityPlusOne; // next-read-pos
                if constexpr ( uses_memcpy ) {
                    if constexpr ( uses_secmem ) {
                        ::explicit_bzero(voidptr_cast(&array[localReadPos]), togo_count*sizeof(Value_type));
                    }
                } else {
                    for(Size_type i=0; i<togo_count; i++) {
                        dtor_one( localReadPos+i );
                    }
                }
                localReadPos = ( localReadPos + togo_count - 1 ) % capacityPlusOne; // last read-pos
            }
            {
                std::unique_lock<std::mutex> lockRead(syncRead); // SC-DRF w/ putImpl via same lock
                readPos = localReadPos; // SC-DRF release atomic readPos
            }
            cvRead.notify_all(); // notify waiting putter, have mutex unlocked before notify_all to avoid pessimistic re-block of notified wait() thread.
            return count;
        }

        bool moveIntoImpl(Value_type &&e, const bool blocking, const fraction_i64& timeout) noexcept {
            if( 1 >= capacityPlusOne ) {
                return false;
            }
            Size_type localWritePos = writePos; // SC-DRF acquire atomic writePos, sync'ing with getImpl
            localWritePos = (localWritePos + 1) % capacityPlusOne;
            if( localWritePos == readPos ) {
                if( blocking ) {
                    interrupted_write = false;
                    std::unique_lock<std::mutex> lockRead(syncRead); // SC-DRF w/ getImpl via same lock
                    const fraction_timespec timeout_time = getMonotonicTime() + fraction_timespec(timeout);
                    while( !interrupted_write && localWritePos == readPos ) {
                        if( fractions_i64::zero == timeout ) {
                            cvRead.wait(lockRead);
                        } else {
                            std::cv_status s = wait_until(cvRead, lockRead, timeout_time );
                            if( std::cv_status::timeout == s && localWritePos == readPos ) {
                                return false;
                            }
                        }
                    }
                    if( interrupted_write ) {
                        interrupted_write = false;
                        return false;
                    }
                } else {
                    return false;
                }
            }
            if constexpr ( is_integral ) {
                array[localWritePos] = e;
            } else if constexpr ( uses_memcpy ) {
                ::memcpy(voidptr_cast(&array[localWritePos]),
                         &e,
                         sizeof(Value_type));
            } else {
                new (const_cast<pointer_mutable>(array + localWritePos)) value_type( std::move(e) ); // placement new
            }
            {
                std::unique_lock<std::mutex> lockWrite(syncWrite); // SC-DRF w/ getImpl via same lock
                writePos = localWritePos; // SC-DRF release atomic writePos
            }
            cvWrite.notify_all(); // notify waiting getter, have mutex unlocked before notify_all to avoid pessimistic re-block of notified wait() thread.
            return true;
        }

        bool copyIntoImpl(const Value_type &e, const bool blocking, const fraction_i64& timeout) noexcept {
            if( !std::is_copy_constructible_v<Value_type> ) {
                ABORT("Value_type is not copy constructible");
                return false;
            }
            if( 1 >= capacityPlusOne ) {
                return false;
            }
            Size_type localWritePos = writePos; // SC-DRF acquire atomic writePos, sync'ing with getImpl
            localWritePos = (localWritePos + 1) % capacityPlusOne;
            if( localWritePos == readPos ) {
                if( blocking ) {
                    interrupted_write = false;
                    std::unique_lock<std::mutex> lockRead(syncRead); // SC-DRF w/ getImpl via same lock
                    const fraction_timespec timeout_time = getMonotonicTime() + fraction_timespec(timeout);
                    while( !interrupted_write && localWritePos == readPos ) {
                        if( fractions_i64::zero == timeout ) {
                            cvRead.wait(lockRead);
                        } else {
                            std::cv_status s = wait_until(cvRead, lockRead, timeout_time );
                            if( std::cv_status::timeout == s && localWritePos == readPos ) {
                                return false;
                            }
                        }
                    }
                    if( interrupted_write ) {
                        interrupted_write = false;
                        return false;
                    }
                } else {
                    return false;
                }
            }
            if constexpr ( is_integral ) {
                array[localWritePos] = e;
            } else if constexpr ( uses_memcpy ) {
                ::memcpy(voidptr_cast(&array[localWritePos]),
                         &e,
                         sizeof(Value_type));
            } else {
                new (const_cast<pointer_mutable>(array + localWritePos)) value_type( e ); // placement new
            }
            {
                std::unique_lock<std::mutex> lockWrite(syncWrite); // SC-DRF w/ getImpl via same lock
                writePos = localWritePos; // SC-DRF release atomic writePos
            }
            cvWrite.notify_all(); // notify waiting getter, have mutex unlocked before notify_all to avoid pessimistic re-block of notified wait() thread.
            return true;
        }

        bool copyIntoImpl(const Value_type *first, const Value_type* last, const bool blocking, const fraction_i64& timeout) noexcept {
            if( !std::is_copy_constructible_v<Value_type> ) {
                ABORT("Value_type is not copy constructible");
                return false;
            }
            const Value_type *iter_in = first;
            const Size_type total_count = last - first;

            if( total_count >= capacityPlusOne ) {
                return false;
            }
            if( 0 == total_count ) {
                return true;
            }

            Size_type localWritePos = writePos; // SC-DRF acquire atomic writePos, sync'ing with getImpl
            Size_type available = freeSlots();
            if( total_count > available ) {
                if( blocking ) {
                    interrupted_write = false;
                    std::unique_lock<std::mutex> lockRead(syncRead); // SC-DRF w/ getImpl via same lock
                    available = freeSlots();
                    const fraction_timespec timeout_time = getMonotonicTime() + fraction_timespec(timeout);
                    while( !interrupted_write && total_count > available ) {
                        if( fractions_i64::zero == timeout ) {
                            cvRead.wait(lockRead);
                            available = freeSlots();
                        } else {
                            std::cv_status s = wait_until(cvRead, lockRead, timeout_time );
                            available = freeSlots();
                            if( std::cv_status::timeout == s && total_count > available ) {
                                return false;
                            }
                        }
                    }
                    if( interrupted_write ) {
                        interrupted_write = false;
                        return false;
                    }
                } else {
                    return false;
                }
            }
            /**
             * Empty [RW][][ ][ ][ ][ ][ ][ ][ ][ ][ ][ ][ ][ ][ ] ; W==R
             * Avail [ ][ ][R][.][.][.][.][W][ ][ ][ ][ ][ ][ ][ ] ; W > R
             * Avail [.][.][.][W][ ][ ][R][.][.][.][.][.][.][.][.] ; W <  R - 1
             * Full  [.][.][.][.][.][W][R][.][.][.][.][.][.][.][.] ; W==R-1
             */
            // Since available > 0, we can exclude Full case.
            Size_type togo_count = total_count;
            const Size_type localReadPos = readPos;
            if( localWritePos >= localReadPos ) { // Empty at any position or W > R case
                // we have a tail
                localWritePos = ( localWritePos + 1 ) % capacityPlusOne; // next-write-pos
                const Size_type tail_count = std::min(togo_count, capacityPlusOne - localWritePos);
                if constexpr ( uses_memcpy ) {
                    ::memcpy(voidptr_cast(&array[localWritePos]),
                             iter_in,
                             tail_count*sizeof(Value_type));
                } else {
                    for(Size_type i=0; i<tail_count; i++) {
                        new (const_cast<pointer_mutable>(array + localWritePos + i)) value_type( iter_in[i] ); // placement new
                    }
                }
                localWritePos = ( localWritePos + tail_count - 1 ) % capacityPlusOne; // last write-pos
                togo_count -= tail_count;
                iter_in += tail_count;
            }
            if( togo_count > 0 ) {
                // we have a head
                localWritePos = ( localWritePos + 1 ) % capacityPlusOne; // next-write-pos
                if constexpr ( uses_memcpy ) {
                    memcpy(voidptr_cast(&array[localWritePos]),
                           iter_in,
                           togo_count*sizeof(Value_type));
                } else {
                    for(Size_type i=0; i<togo_count; i++) {
                        new (const_cast<pointer_mutable>(array + localWritePos + i)) value_type( iter_in[i] ); // placement new
                    }
                }
                localWritePos = ( localWritePos + togo_count - 1 ) % capacityPlusOne; // last write-pos
            }
            {
                std::unique_lock<std::mutex> lockWrite(syncWrite); // SC-DRF w/ getImpl via same lock
                writePos = localWritePos; // SC-DRF release atomic writePos
            }
            cvWrite.notify_all(); // notify waiting getter, have mutex unlocked before notify_all to avoid pessimistic re-block of notified wait() thread.
            return true;
        }

        void recapacityImpl(const Size_type newCapacity) {
            const Size_type size_ = size();

            if( capacityPlusOne == newCapacity+1 ) {
                return;
            }
            if( size_ > newCapacity ) {
                throw IllegalArgumentException("amount "+std::to_string(newCapacity)+" < size, "+toString(), E_FILE_LINE);
            }

            // save current data
            const Size_type oldCapacityPlusOne = capacityPlusOne;
            Value_type * oldArray = array;
            Size_type oldReadPos = readPos;

            // new blank resized array, starting at position 0
            capacityPlusOne = newCapacity + 1;
            array = newArray(capacityPlusOne);
            readPos = 0;
            writePos = 0;

            // copy saved data
            if( nullptr != oldArray && 0 < size_ ) {
                Size_type localWritePos = writePos;
                for(Size_type i=0; i<size_; i++) {
                    localWritePos = (localWritePos + 1) % capacityPlusOne;
                    oldReadPos = (oldReadPos + 1) % oldCapacityPlusOne;
                    new (const_cast<pointer_mutable>( array + localWritePos )) value_type( std::move( oldArray[oldReadPos] ) ); // placement new
                    dtor_one( oldArray + oldReadPos ); // manual destruction, even after std::move (object still exists)
                }
                writePos = localWritePos;
            }
            freeArray(&oldArray, oldCapacityPlusOne); // and release
        }

        void closeImpl(const bool zeromem) noexcept {
            if( zeromem ) {
                clearAndZeroMemImpl();
            } else {
                clearImpl();
            }
            freeArray(&array, capacityPlusOne);

            capacityPlusOne = 1;
            array = newArray(capacityPlusOne);
            readPos = 0;
            writePos = 0;
        }

    public:

        /** Returns a short string representation incl. size/capacity and internal r/w index (impl. dependent). */
        std::string toString() const noexcept {
            const std::string e_s = isEmpty() ? ", empty" : "";
            const std::string f_s = isFull() ? ", full" : "";
            const std::string mode_s = getMultiPCEnabled() ? ", mpc" : ", one";
            return "ringbuffer<?>[size "+std::to_string(size())+" / "+std::to_string(capacityPlusOne-1)+
                    ", writePos "+std::to_string(writePos)+", readPos "+std::to_string(readPos)+e_s+f_s+mode_s+"]";
        }

        /** Debug functionality - Dumps the contents of the internal array. */
        void dump(FILE *stream, std::string prefix) const noexcept {
            fprintf(stream, "%s %s, array %p\n", prefix.c_str(), toString().c_str(), array);
        }

        constexpr_cxx20 std::string get_info() const noexcept {
            const std::string e_s = isEmpty() ? ", empty" : "";
            const std::string f_s = isFull() ? ", full" : "";
            const std::string mode_s = getMultiPCEnabled() ? ", mpc" : ", one";
            std::string res("ringbuffer<?>[this "+jau::to_hexstring(this)+
                            ", size "+std::to_string(size())+" / "+std::to_string(capacityPlusOne-1)+
                            ", "+e_s+f_s+mode_s+", type[integral "+std::to_string(is_integral)+
                            ", trivialCpy "+std::to_string(std::is_trivially_copyable_v<Value_type>)+
                            "], uses[mmove "+std::to_string(uses_memmove)+
                            ", mcpy "+std::to_string(uses_memcpy)+
                            ", smem "+std::to_string(uses_secmem)+
                            "]]");
            return res;
        }

        /**
         * Create a full ring buffer instance w/ the given array's net capacity and content.
         * <p>
         * Example for a 10 element Integer array:
         * <pre>
         *  Integer[] source = new Integer[10];
         *  // fill source with content ..
         *  ringbuffer<Integer> rb = new ringbuffer<Integer>(source);
         * </pre>
         * </p>
         * <p>
         * {@link #isFull()} returns true on the newly created full ring buffer.
         * </p>
         * <p>
         * Implementation will allocate an internal array with size of array `copyFrom` <i>plus one</i>,
         * and copy all elements from array `copyFrom` into the internal array.
         * </p>
         * @param copyFrom mandatory source array determining ring buffer's net {@link #capacity()} and initial content.
         * @throws IllegalArgumentException if `copyFrom` is `nullptr`
         */
        ringbuffer(const std::vector<Value_type> & copyFrom) noexcept
        : capacityPlusOne(copyFrom.size() + 1), array(newArray(capacityPlusOne)),
          readPos(0), writePos(0)
        {
            resetImpl(copyFrom.data(), copyFrom.size());
            _DEBUG_DUMP("ctor(vector<Value_type>)");
        }

        /**
         * @param copyFrom
         * @param copyFromSize
         */
        ringbuffer(const Value_type * copyFrom, const Size_type copyFromSize) noexcept
        : capacityPlusOne(copyFromSize + 1), array(newArray(capacityPlusOne)),
          readPos(0), writePos(0)
        {
            resetImpl(copyFrom, copyFromSize);
            _DEBUG_DUMP("ctor(Value_type*, len)");
        }

        /**
         * Create an empty ring buffer instance w/ the given net `capacity`.
         * <p>
         * Example for a 10 element Integer array:
         * <pre>
         *  ringbuffer<Integer> rb = new ringbuffer<Integer>(10, Integer[].class);
         * </pre>
         * </p>
         * <p>
         * {@link #isEmpty()} returns true on the newly created empty ring buffer.
         * </p>
         * <p>
         * Implementation will allocate an internal array of size `capacity` <i>plus one</i>.
         * </p>
         * @param arrayType the array type of the created empty internal array.
         * @param capacity the initial net capacity of the ring buffer
         */
        ringbuffer(const Size_type capacity) noexcept
        : capacityPlusOne(capacity + 1), array(newArray(capacityPlusOne)),
          readPos(0), writePos(0)
        {
            _DEBUG_DUMP("ctor(capacity)");
        }

        ~ringbuffer() noexcept {
            _DEBUG_DUMP("dtor(def)");
            if( nullptr != array ) {
                clearImpl();
                freeArray(&array, capacityPlusOne);
            }
        }

        ringbuffer(const ringbuffer &_source) noexcept
        : capacityPlusOne(_source.capacityPlusOne), array(newArray(capacityPlusOne)),
          readPos(0), writePos(0)
        {
            std::unique_lock<std::mutex> lockMultiReadS(_source.syncMultiRead, std::defer_lock); // utilize std::lock(r, w), allowing mixed order waiting on read/write ops
            std::unique_lock<std::mutex> lockMultiWriteS(_source.syncMultiWrite, std::defer_lock); // otherwise RAII-style relinquish via destructor
            std::lock(lockMultiReadS, lockMultiWriteS);                                          // *this instance does not exist yet
            cloneFrom(false, _source);
            _DEBUG_DUMP("ctor(copy.this)");
            _DEBUG_DUMP2(_source, "ctor(copy.source)");
        }

        ringbuffer& operator=(const ringbuffer &_source) noexcept {
            if( this == &_source ) {
                return *this;
            }
            if( multi_pc_enabled ) {
                std::unique_lock<std::mutex> lockMultiReadS(_source.syncMultiRead, std::defer_lock); // utilize std::lock(r, w), allowing mixed order waiting on read/write ops
                std::unique_lock<std::mutex> lockMultiWriteS(_source.syncMultiWrite, std::defer_lock); // otherwise RAII-style relinquish via destructor
                std::unique_lock<std::mutex> lockMultiRead(syncMultiRead, std::defer_lock);          // same for *this instance!
                std::unique_lock<std::mutex> lockMultiWrite(syncMultiWrite, std::defer_lock);
                std::lock(lockMultiReadS, lockMultiWriteS, lockMultiRead, lockMultiWrite);

                return assignCopyImpl(_source);
            } else {
                std::unique_lock<std::mutex> lockReadS(_source.syncRead, std::defer_lock); // utilize std::lock(r, w), allowing mixed order waiting on read/write ops
                std::unique_lock<std::mutex> lockWriteS(_source.syncWrite, std::defer_lock); // otherwise RAII-style relinquish via destructor
                std::unique_lock<std::mutex> lockRead(syncRead, std::defer_lock);          // same for *this instance!
                std::unique_lock<std::mutex> lockWrite(syncWrite, std::defer_lock);
                std::lock(lockReadS, lockWriteS, lockRead, lockWrite);

                return assignCopyImpl(_source);
            }
        }

        ringbuffer(ringbuffer &&o) noexcept = default;
        ringbuffer& operator=(ringbuffer &&o) noexcept = default;

        /**
         * Return whether multiple producer and consumer are enabled,
         * see @ref ringbuffer_multi_pc and @ref ringbuffer_single_pc.
         *
         * Defaults to `false`.
         * @see @ref ringbuffer_multi_pc
         * @see @ref ringbuffer_single_pc
         * @see getMultiPCEnabled()
         * @see setMultiPCEnabled()
         */
        constexpr bool getMultiPCEnabled() const { return multi_pc_enabled; }

        /**
         * Enable or disable capability to handle multiple producer and consumer,
         * see @ref ringbuffer_multi_pc and @ref ringbuffer_single_pc.
         *
         * Defaults to `false`.
         * @see @ref ringbuffer_multi_pc
         * @see @ref ringbuffer_single_pc
         * @see getMultiPCEnabled()
         * @see setMultiPCEnabled()
         */
        constexpr_non_literal_var void setMultiPCEnabled(const bool v) {
            /**
             * Using just 'constexpr_non_literal_var' because
             * clang: 'unique_lock<std::mutex>' is not literal because it is not an aggregate and has no constexpr constructors other than copy or move constructors
             */
            if( multi_pc_enabled ) {
                std::unique_lock<std::mutex> lockMultiRead(syncMultiRead, std::defer_lock);          // same for *this instance!
                std::unique_lock<std::mutex> lockMultiWrite(syncMultiWrite, std::defer_lock);
                std::unique_lock<std::mutex> lockRead(syncRead, std::defer_lock);          // same for *this instance!
                std::unique_lock<std::mutex> lockWrite(syncWrite, std::defer_lock);
                std::lock(lockMultiRead, lockMultiWrite, lockRead, lockWrite);

                multi_pc_enabled=v;
            } else {
                std::unique_lock<std::mutex> lockRead(syncRead, std::defer_lock);          // same for *this instance!
                std::unique_lock<std::mutex> lockWrite(syncWrite, std::defer_lock);
                std::lock(lockRead, lockWrite);

                multi_pc_enabled=v;
            }
        }

        /** Returns the net capacity of this ring buffer. */
        Size_type capacity() const noexcept { return capacityPlusOne - 1; }

        /** Returns the number of free slots available to put.  */
        Size_type freeSlots() const noexcept { return capacityPlusOne - 1 - size(); }

        /** Returns true if this ring buffer is empty, otherwise false. */
        bool isEmpty() const noexcept { return writePos == readPos; /* 0 == size */ }

        /** Returns true if this ring buffer is full, otherwise false. */
        bool isFull() const noexcept { return ( writePos + 1 ) % capacityPlusOne == readPos; /* W == R - 1 */; }

        /** Returns the number of elements in this ring buffer. */
        Size_type size() const noexcept {
            const Size_type R = readPos;
            const Size_type W = writePos;
            // W >= R: W - R
            // W <  R: C+1 - R - 1 + W + 1 = C+1 - R + W
            return W >= R ? W - R : capacityPlusOne - R + W;
        }

        /**
         * Blocks until at least `count` elements have been put
         * for subsequent get() and getBlocking().
         *
         * @param min_count minimum number of put slots
         * @param timeout maximum duration in fractions of seconds to wait, where fractions_i64::zero waits infinitely
         * @return the number of put elements, available for get() and getBlocking()
         */
        Size_type waitForElements(const Size_type min_count, const fraction_i64& timeout) noexcept {
            if( multi_pc_enabled ) {
                std::unique_lock<std::mutex> lockMultiRead(syncMultiRead); // acquire syncMultiRead, _not_ sync'ing w/ putImpl
                return waitForElementsImpl(min_count, timeout);
            } else {
                return waitForElementsImpl(min_count, timeout);
            }
        }

        /**
         * Blocks until at least `count` free slots become available
         * for subsequent put() and putBlocking().
         *
         * @param min_count minimum number of free slots
         * @param timeout maximum duration in fractions of seconds to wait, where fractions_i64::zero waits infinitely
         * @return the number of free slots, available for put() and putBlocking()
         */
        Size_type waitForFreeSlots(const Size_type min_count, const fraction_i64& timeout) noexcept {
            if( multi_pc_enabled ) {
                std::unique_lock<std::mutex> lockMultiWrite(syncMultiWrite); // acquire syncMultiWrite, _not_ sync'ing w/ getImpl
                return waitForFreeSlotsImpl(min_count, timeout);
            } else {
                return waitForFreeSlotsImpl(min_count, timeout);
            }
        }

        /**
         * Releasing all elements available, i.e. size() at the time of the call
         *
         * It is the user's obligation to ensure thread safety when using @ref ringbuffer_single_pc,
         * as implementation can only synchronize on the blocked put() and get() std::mutex.
         *
         * Assuming no concurrent put() operation, after the call:
         * - {@link #isEmpty()} will return `true`
         * - {@link #size()} shall return `0`
         *
         * @param zeromem pass true to zero ringbuffer memory after releasing elements, otherwise non template type parameter use_secmem determines the behavior (default), see @ref ringbuffer_ntt_params.
         * @see @ref ringbuffer_ntt_params
         */
        void clear(const bool zeromem=false) noexcept {
            if( multi_pc_enabled ) {
                std::unique_lock<std::mutex> lockMultiRead(syncMultiRead, std::defer_lock);          // same for *this instance!
                std::unique_lock<std::mutex> lockMultiWrite(syncMultiWrite, std::defer_lock);
                std::unique_lock<std::mutex> lockRead(syncRead, std::defer_lock);          // same for *this instance!
                std::unique_lock<std::mutex> lockWrite(syncWrite, std::defer_lock);
                std::lock(lockMultiRead, lockMultiWrite, lockRead, lockWrite);

                if( zeromem ) {
                    clearAndZeroMemImpl();
                } else {
                    clearImpl();
                }
            } else {
                std::unique_lock<std::mutex> lockRead(syncRead, std::defer_lock);          // same for *this instance!
                std::unique_lock<std::mutex> lockWrite(syncWrite, std::defer_lock);
                std::lock(lockRead, lockWrite);

                if( zeromem ) {
                    clearAndZeroMemImpl();
                } else {
                    clearImpl();
                }
            }
            cvRead.notify_all(); // notify waiting writer, have mutex unlocked before notify_all to avoid pessimistic re-block of notified wait() thread.
        }

        /**
         * Close this ringbuffer by releasing all elements available
         * and resizing capacity to zero.
         * Essentially causing all read and write operations to fail.
         *
         * Potential writer and reader thread will be
         * - notified, allowing them to be woken up
         * - interrupted to abort the write and read operation
         *
         * Subsequent write and read operations will fail and not block.
         *
         * After the call:
         * - {@link #isEmpty()} will return `true`
         * - {@link #size()} shall return `0`
         * - {@link #capacity()} shall return `0`
         * - {@link #freeSlots()} shall return `0`
         */
        void close(const bool zeromem=false) noexcept {
            if( multi_pc_enabled ) {
                std::unique_lock<std::mutex> lockMultiRead(syncMultiRead, std::defer_lock);          // same for *this instance!
                std::unique_lock<std::mutex> lockMultiWrite(syncMultiWrite, std::defer_lock);
                std::unique_lock<std::mutex> lockRead(syncRead, std::defer_lock);          // same for *this instance!
                std::unique_lock<std::mutex> lockWrite(syncWrite, std::defer_lock);
                std::lock(lockMultiRead, lockMultiWrite, lockRead, lockWrite);

                closeImpl(zeromem);
                interrupted_read = true;
                interrupted_write = true;
            } else {
                std::unique_lock<std::mutex> lockRead(syncRead, std::defer_lock);          // same for *this instance!
                std::unique_lock<std::mutex> lockWrite(syncWrite, std::defer_lock);
                std::lock(lockRead, lockWrite);

                closeImpl(zeromem);
                interrupted_write = true;
                interrupted_read = true;
            }
            // have mutex unlocked before notify_all to avoid pessimistic re-block of notified wait() thread.
            cvRead.notify_all();  // notify waiting writer
            cvWrite.notify_all(); // notify waiting reader
        }

        /**
         * Clears all elements and add all `copyFrom` elements thereafter, as if reconstructing this ringbuffer instance.
         *
         * It is the user's obligation to ensure thread safety when using @ref ringbuffer_single_pc,
         * as implementation can only synchronize on the blocked put() and get() std::mutex.
         *
         * @param copyFrom Mandatory array w/ length {@link #capacity()} to be copied into the internal array.
         *
         * @see getMultiPCEnabled()
         * @see setMultiPCEnabled()
         * @see @ref ringbuffer_multi_pc
         * @see @ref ringbuffer_single_pc
         */
        void reset(const Value_type * copyFrom, const Size_type copyFromCount) noexcept {
            if( multi_pc_enabled ) {
                std::unique_lock<std::mutex> lockMultiRead(syncMultiRead, std::defer_lock);          // same for *this instance!
                std::unique_lock<std::mutex> lockMultiWrite(syncMultiWrite, std::defer_lock);
                std::unique_lock<std::mutex> lockRead(syncRead, std::defer_lock);          // same for *this instance!
                std::unique_lock<std::mutex> lockWrite(syncWrite, std::defer_lock);
                std::lock(lockMultiRead, lockMultiWrite, lockRead, lockWrite);

                resetImpl(copyFrom, copyFromCount);
            } else {
                std::unique_lock<std::mutex> lockRead(syncRead, std::defer_lock);          // same for *this instance!
                std::unique_lock<std::mutex> lockWrite(syncWrite, std::defer_lock);
                std::lock(lockRead, lockWrite);

                resetImpl(copyFrom, copyFromCount);
            }
        }

        /**
         * Clears all elements and add all `copyFrom` elements thereafter, as if reconstructing this ringbuffer instance.
         *
         * It is the user's obligation to ensure thread safety when using @ref ringbuffer_single_pc,
         * as implementation can only synchronize on the blocked put() and get() std::mutex.
         *
         * @param copyFrom
         *
         * @see getMultiPCEnabled()
         * @see setMultiPCEnabled()
         * @see @ref ringbuffer_multi_pc
         * @see @ref ringbuffer_single_pc
         */
        void reset(const std::vector<Value_type> & copyFrom) noexcept {
            if( multi_pc_enabled ) {
                std::unique_lock<std::mutex> lockMultiRead(syncMultiRead, std::defer_lock);          // same for *this instance!
                std::unique_lock<std::mutex> lockMultiWrite(syncMultiWrite, std::defer_lock);
                std::unique_lock<std::mutex> lockRead(syncRead, std::defer_lock);          // same for *this instance!
                std::unique_lock<std::mutex> lockWrite(syncWrite, std::defer_lock);
                std::lock(lockMultiRead, lockMultiWrite, lockRead, lockWrite);

                resetImpl(copyFrom.data(), copyFrom.size());
            } else {
                std::unique_lock<std::mutex> lockRead(syncRead, std::defer_lock);          // same for *this instance!
                std::unique_lock<std::mutex> lockWrite(syncWrite, std::defer_lock);
                std::lock(lockRead, lockWrite);

                resetImpl(copyFrom.data(), copyFrom.size());
            }
        }

        /**
         * Resizes this ring buffer's capacity.
         *
         * New capacity must be greater than current size.
         *
         * It is the user's obligation to ensure thread safety when using @ref ringbuffer_single_pc,
         * as implementation can only synchronize on the blocked put() and get() std::mutex.
         *
         * @param newCapacity
         *
         * @see getMultiPCEnabled()
         * @see setMultiPCEnabled()
         * @see @ref ringbuffer_multi_pc
         * @see @ref ringbuffer_single_pc
         */
        void recapacity(const Size_type newCapacity) {
            if( multi_pc_enabled ) {
                std::unique_lock<std::mutex> lockMultiRead(syncMultiRead, std::defer_lock);          // same for *this instance!
                std::unique_lock<std::mutex> lockMultiWrite(syncMultiWrite, std::defer_lock);
                std::unique_lock<std::mutex> lockRead(syncRead, std::defer_lock);          // same for *this instance!
                std::unique_lock<std::mutex> lockWrite(syncWrite, std::defer_lock);
                std::lock(lockMultiRead, lockMultiWrite, lockRead, lockWrite);
                recapacityImpl(newCapacity);
            } else {
                std::unique_lock<std::mutex> lockRead(syncRead, std::defer_lock);          // same for *this instance!
                std::unique_lock<std::mutex> lockWrite(syncWrite, std::defer_lock);
                std::lock(lockRead, lockWrite);

                recapacityImpl(newCapacity);
            }
        }

        /**
         * Interrupt a potentially blocked reader.
         *
         * Call this method if intended to abort reading and to interrupt the reader thread's potentially blocked read-access call.
         */
        void interruptReader() noexcept { interrupted_read = true; cvWrite.notify_all(); }

        /**
         * Interrupt a potentially blocked writer.
         *
         * Call this method if intended to abort writing and to interrupt the writing thread's potentially blocked write-access call.
         */
        void interruptWriter() noexcept { interrupted_write = true; cvRead.notify_all(); }

        /**
         * Peeks the next element at the read position w/o modifying pointer, nor blocking.
         *
         * Method is non blocking and returns immediately;.
         *
         * @param result storage for the resulting value if successful, otherwise unchanged.
         * @return true if successful, otherwise false.
         */
        bool peek(Value_type& result) noexcept {
            if( multi_pc_enabled ) {
                std::unique_lock<std::mutex> lockMultiRead(syncMultiRead); // acquire syncMultiRead, _not_ sync'ing w/ putImpl
                return peekImpl(result, false, fractions_i64::zero);
            } else {
                return peekImpl(result, false, fractions_i64::zero);
            }
        }

        /**
         * Peeks the next element at the read position w/o modifying pointer, but with blocking.
         *
         * @param result storage for the resulting value if successful, otherwise unchanged.
         * @param timeout maximum duration in fractions of seconds to wait, where fractions_i64::zero waits infinitely
         * @return true if successful, otherwise false.
         */
        bool peekBlocking(Value_type& result, const fraction_i64& timeout) noexcept {
            if( multi_pc_enabled ) {
                std::unique_lock<std::mutex> lockMultiRead(syncMultiRead); // acquire syncMultiRead, _not_ sync'ing w/ putImpl
                return peekImpl(result, true, timeout);
            } else {
                return peekImpl(result, true, timeout);
            }
        }

        /**
         * Dequeues the oldest enqueued element, if available.
         *
         * The ring buffer slot will be released and its value moved to the caller's `result` storage, if successful.
         *
         * Method is non blocking and returns immediately;.
         *
         * @param result storage for the resulting value if successful, otherwise unchanged.
         * @return true if successful, otherwise false.
         */
        bool get(Value_type& result) noexcept {
            if( multi_pc_enabled ) {
                std::unique_lock<std::mutex> lockMultiRead(syncMultiRead); // acquire syncMultiRead, _not_ sync'ing w/ putImpl
                return moveOutImpl(result, false, fractions_i64::zero);
            } else {
                return moveOutImpl(result, false, fractions_i64::zero);
            }
        }

        /**
         * Dequeues the oldest enqueued element.
         *
         * The ring buffer slot will be released and its value moved to the caller's `result` storage, if successful.
         *
         * @param result storage for the resulting value if successful, otherwise unchanged.
         * @param timeout maximum duration in fractions of seconds to wait, where fractions_i64::zero waits infinitely
         * @return true if successful, otherwise false.
         */
        bool getBlocking(Value_type& result, const fraction_i64& timeout) noexcept {
            if( multi_pc_enabled ) {
                std::unique_lock<std::mutex> lockMultiRead(syncMultiRead); // acquire syncMultiRead, _not_ sync'ing w/ putImpl
                return moveOutImpl(result, true, timeout);
            } else {
                return moveOutImpl(result, true, timeout);
            }
        }

        /**
         * Dequeues the oldest enqueued `min(dest_len, size()>=min_count)` elements by copying them into the given consecutive 'dest' storage.
         *
         * The ring buffer slots will be released and its value moved to the caller's `dest` storage, if successful.
         *
         * Method is non blocking and returns immediately;.
         *
         * @param dest pointer to first storage element of `dest_len` consecutive elements to store the values, if successful.
         * @param dest_len number of consecutive elements in `dest`, hence maximum number of elements to return.
         * @param min_count minimum number of consecutive elements to return.
         * @return actual number of elements returned
         */
        Size_type get(Value_type *dest, const Size_type dest_len, const Size_type min_count) noexcept {
            if( multi_pc_enabled ) {
                std::unique_lock<std::mutex> lockMultiRead(syncMultiRead); // acquire syncMultiRead, _not_ sync'ing w/ putImpl
                return moveOutImpl(dest, dest_len, min_count, false, fractions_i64::zero);
            } else {
                return moveOutImpl(dest, dest_len, min_count, false, fractions_i64::zero);
            }
        }

        /**
         * Dequeues the oldest enqueued `min(dest_len, size()>=min_count)` elements by copying them into the given consecutive 'dest' storage.
         *
         * The ring buffer slots will be released and its value moved to the caller's `dest` storage, if successful.
         *
         * @param dest pointer to first storage element of `dest_len` consecutive elements to store the values, if successful.
         * @param dest_len number of consecutive elements in `dest`, hence maximum number of elements to return.
         * @param min_count minimum number of consecutive elements to return
         * @param timeout maximum duration in fractions of seconds to wait, where fractions_i64::zero waits infinitely
         * @return actual number of elements returned
         */
        Size_type getBlocking(Value_type *dest, const Size_type dest_len, const Size_type min_count, const fraction_i64& timeout) noexcept {
            if( multi_pc_enabled ) {
                std::unique_lock<std::mutex> lockMultiRead(syncMultiRead); // acquire syncMultiRead, _not_ sync'ing w/ putImpl
                return moveOutImpl(dest, dest_len, min_count, true, timeout);
            } else {
                return moveOutImpl(dest, dest_len, min_count, true, timeout);
            }
        }

        /**
         * Drops up to `max_count` oldest enqueued elements,
         * but may drop less if not available.
         *
         * Method is non blocking and returns immediately;.
         *
         * @param max_count maximum number of elements to drop from ringbuffer.
         * @return number of elements dropped
         */
        Size_type drop(const Size_type max_count) noexcept {
            if( multi_pc_enabled ) {
                std::unique_lock<std::mutex> lockMultiRead(syncMultiRead, std::defer_lock);          // utilize std::lock(r, w), allowing mixed order waiting on read/write ops
                std::unique_lock<std::mutex> lockMultiWrite(syncMultiWrite, std::defer_lock);        // otherwise RAII-style relinquish via destructor
                std::lock(lockMultiRead, lockMultiWrite);
                return dropImpl(max_count, false, fractions_i64::zero);
            } else {
                return dropImpl(max_count, false, fractions_i64::zero);
            }
        }

        /**
         * Drops exactly `count` oldest enqueued elements,
         * will block until they become available.
         *
         * In `count` elements are not available to drop even after
         * blocking for `timeoutMS`, no element will be dropped.
         *
         * @param count number of elements to drop from ringbuffer.
         * @param timeout maximum duration in fractions of seconds to wait, where fractions_i64::zero waits infinitely
         * @return true if successful, otherwise false
         */
        bool dropBlocking(const Size_type count, const fraction_i64& timeout) noexcept {
            if( multi_pc_enabled ) {
                std::unique_lock<std::mutex> lockMultiRead(syncMultiRead, std::defer_lock);          // utilize std::lock(r, w), allowing mixed order waiting on read/write ops
                std::unique_lock<std::mutex> lockMultiWrite(syncMultiWrite, std::defer_lock);        // otherwise RAII-style relinquish via destructor
                std::lock(lockMultiRead, lockMultiWrite);
                return 0 != dropImpl(count, true, timeout);
            } else {
                return 0 != dropImpl(count, true, timeout);
            }
        }

        /**
         * Enqueues the given element by moving it into this ringbuffer storage.
         *
         * Returns true if successful, otherwise false in case buffer is full.
         *
         * Method is non blocking and returns immediately;.
         *
         * @return true if successful, otherwise false
         */
        bool put(Value_type && e) noexcept {
            if( multi_pc_enabled ) {
                std::unique_lock<std::mutex> lockMultiWrite(syncMultiWrite); // acquire syncMultiWrite, _not_ sync'ing w/ getImpl
                return moveIntoImpl(std::move(e), false, fractions_i64::zero);
            } else {
                return moveIntoImpl(std::move(e), false, fractions_i64::zero);
            }
        }

        /**
         * Enqueues the given element by moving it into this ringbuffer storage.
         *
         * @param e
         * @param timeout maximum duration in fractions of seconds to wait, where fractions_i64::zero waits infinitely
         * @return true if successful, otherwise false in case timeout occurred or otherwise.
         */
        bool putBlocking(Value_type && e, const fraction_i64& timeout) noexcept {
            if( multi_pc_enabled ) {
                std::unique_lock<std::mutex> lockMultiWrite(syncMultiWrite); // acquire syncMultiWrite, _not_ sync'ing w/ getImpl
                return moveIntoImpl(std::move(e), true, timeout);
            } else {
                return moveIntoImpl(std::move(e), true, timeout);
            }
        }

        /**
         * Enqueues the given element by copying it into this ringbuffer storage.
         *
         * Returns true if successful, otherwise false in case buffer is full.
         *
         * Method is non blocking and returns immediately;.
         *
         * @return true if successful, otherwise false
         */
        bool put(const Value_type & e) noexcept {
            if( multi_pc_enabled ) {
                std::unique_lock<std::mutex> lockMultiWrite(syncMultiWrite); // acquire syncMultiWrite, _not_ sync'ing w/ getImpl
                return copyIntoImpl(e, false, fractions_i64::zero);
            } else {
                return copyIntoImpl(e, false, fractions_i64::zero);
            }
        }

        /**
         * Enqueues the given element by copying it into this ringbuffer storage.
         *
         * @param timeout maximum duration in fractions of seconds to wait, where fractions_i64::zero waits infinitely
         * @return true if successful, otherwise false in case timeout occurred or otherwise.
         */
        bool putBlocking(const Value_type & e, const fraction_i64& timeout) noexcept {
            if( multi_pc_enabled ) {
                std::unique_lock<std::mutex> lockMultiWrite(syncMultiWrite); // acquire syncMultiWrite, _not_ sync'ing w/ getImpl
                return copyIntoImpl(e, true, timeout);
            } else {
                return copyIntoImpl(e, true, timeout);
            }
        }

        /**
         * Enqueues the given range of consecutive elements by copying it into this ringbuffer storage.
         *
         * Returns true if successful, otherwise false in case buffer is full.
         *
         * Method is non blocking and returns immediately;.
         *
         * @param first pointer to first consecutive element to range of value_type [first, last)
         * @param last pointer to last consecutive element to range of value_type [first, last)
         * @return true if successful, otherwise false
         */
        bool put(const Value_type *first, const Value_type* last) noexcept {
            if( multi_pc_enabled ) {
                std::unique_lock<std::mutex> lockMultiWrite(syncMultiWrite); // acquire syncMultiWrite, _not_ sync'ing w/ getImpl
                return copyIntoImpl(first, last, false, fractions_i64::zero);
            } else {
                return copyIntoImpl(first, last, false, fractions_i64::zero);
            }
        }

        /**
         * Enqueues the given range of consecutive elementa by copying it into this ringbuffer storage.
         *
         * @param first pointer to first consecutive element to range of value_type [first, last)
         * @param last pointer to last consecutive element to range of value_type [first, last)
         * @param timeout maximum duration in fractions of seconds to wait, where fractions_i64::zero waits infinitely
         * @return true if successful, otherwise false in case timeout occurred or otherwise.
         */
        bool putBlocking(const Value_type *first, const Value_type* last, const fraction_i64& timeout) noexcept {
            if( multi_pc_enabled ) {
                std::unique_lock<std::mutex> lockMultiWrite(syncMultiWrite); // acquire syncMultiWrite, _not_ sync'ing w/ getImpl
                return copyIntoImpl(first, last, true, timeout);
            } else {
                return copyIntoImpl(first, last, true, timeout);
            }
        }
};

/**@}*/

} /* namespace jau */

/** \example test_lfringbuffer01.cpp
 * This C++ unit test validates jau::ringbuffer w/o parallel processing.
 * <p>
 * With test_lfringbuffer11.cpp, this work verifies jau::ringbuffer correctness
 * </p>
 */

/** \example test_lfringbuffer11.cpp
 * This C++ unit test validates jau::ringbuffer with parallel processing.
 * <p>
 * With test_lfringbuffer01.cpp, this work verifies jau::ringbuffer correctness
 * </p>
 */

#endif /* JAU_RINGBUFFER_HPP_ */
