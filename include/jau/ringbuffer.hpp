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

/**
 * Ring buffer implementation, a.k.a circular buffer,
 * exposing <i>lock-free</i>
 * {@link #get() get*(..)} and {@link #put(Object) put*(..)} methods.
 * <p>
 * Implementation utilizes the <i>Always Keep One Slot Open</i>,
 * hence implementation maintains an internal array of <code>capacity</code> <i>plus one</i>!
 * </p>
 * <p>
 * Implementation is thread safe if:
 * <ul>
 *   <li>{@link #get() get*(..)} operations from multiple threads.</li>
 *   <li>{@link #put(Object) put*(..)} operations from multiple threads.</li>
 *   <li>{@link #get() get*(..)} and {@link #put(Object) put*(..)} thread may be the same.</li>
 * </ul>
 * </p>
 * <p>
 * Following methods acquire the global multi-read _and_ -write mutex:
 * <ul>
 *  <li>{@link #resetFull(Object[])}</li>
 *  <li>{@link #clear()}</li>
 *  <li>{@link #growEmptyBuffer(Object[])}</li>
 * </ul>
 * </p>
 * <p>
 * Characteristics:
 * <ul>
 *   <li>Read position points to the last read element.</li>
 *   <li>Write position points to the last written element.</li>
 * </ul>
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
 * </p>
 * See also:
 * <pre>
 * - Sequentially Consistent (SC) ordering or SC-DRF (data race free) <https://en.cppreference.com/w/cpp/atomic/memory_order#Sequentially-consistent_ordering>
 * - std::memory_order <https://en.cppreference.com/w/cpp/atomic/memory_order>
 * </pre>
 * <p>
 * We would like to pass `NullValue_type nullelem` as a non-type template parameter of type `NullValue_type`, a potential Class.
 * However, this is only allowed in C++20 and we use C++17 for now.
 * Hence we have to pass `NullValue_type nullelem` in the constructor.
 * </p>
 * @see jau::sc_atomic_critical
 */
template <typename Value_type, typename NullValue_type, typename Size_type,
          bool use_memcpy = std::is_trivially_copyable_v<Value_type>,
          bool use_memset = std::is_integral_v<Value_type> && sizeof(Value_type)==1 &&
                            std::is_integral_v<NullValue_type> && sizeof(NullValue_type)==1
         >
class ringbuffer {
    public:
        constexpr static const bool uses_memcpy = use_memcpy;
        constexpr static const bool uses_memset = use_memset;

        // typedefs' for C++ named requirements: Container (ex iterator)

        typedef Value_type                                  value_type;
        typedef value_type*                                 pointer;
        typedef const value_type*                           const_pointer;
        typedef value_type&                                 reference;
        typedef const value_type&                           const_reference;
        typedef Size_type                                   size_type;
        typedef typename std::make_signed<size_type>::type  difference_type;

    private:
        /** SC atomic integral scalar jau::nsize_t. Memory-Model (MM) guaranteed sequential consistency (SC) between acquire (read) and release (write) */
        typedef ordered_atomic<Size_type, std::memory_order::memory_order_seq_cst> sc_atomic_Size_type;

        /** Relaxed non-SC atomic integral scalar jau::nsize_t. Memory-Model (MM) only guarantees the atomic value, _no_ sequential consistency (SC) between acquire (read) and release (write). */
        typedef ordered_atomic<Size_type, std::memory_order::memory_order_relaxed> relaxed_atomic_Size_type;

        /** synchronizes write-operations (put*), i.e. modifying the writePos. */
        mutable std::mutex syncWrite, syncMultiWrite; // Memory-Model (MM) guaranteed sequential consistency (SC) between acquire and release
        std::condition_variable cvWrite;

        /** synchronizes read-operations (get*), i.e. modifying the readPos. */
        mutable std::mutex syncRead,  syncMultiRead;  // Memory-Model (MM) guaranteed sequential consistency (SC) between acquire and release
        std::condition_variable cvRead;

        /* const */ NullValue_type nullelem;    // not final due to assignment operation
        /* const */ Size_type capacityPlusOne;  // not final due to grow
        /* const */ Value_type * array;         // Synchronized due to MM's data-race-free SC (SC-DRF) between [atomic] acquire/release
        sc_atomic_Size_type readPos;     // Memory-Model (MM) guaranteed sequential consistency (SC) between acquire (read) and release (write)
        sc_atomic_Size_type writePos;    // ditto

        // DBG_PRINT("");
        Value_type * newArray(const Size_type count) noexcept {
#if 0
            Value_type *r = new Value_type[count];
            _DEBUG_DUMP("newArray ...");
            _DEBUG_PRINT("newArray %" PRIu64 "\n", count);
            return r;
#else
            return new Value_type[count];
#endif
        }
        void freeArray(Value_type ** a) noexcept {
            _DEBUG_DUMP("freeArray(def)");
            _DEBUG_PRINT("freeArray %p\n", *a);
            if( nullptr == *a ) {
                ABORT("ringbuffer::freeArray with nullptr");
            }
            delete[] *a;
            *a = nullptr;
        }

        template<typename _DataType, typename _NullType>
        static void* memset_wrap(_DataType *block, const _NullType& c, size_t n,
                std::enable_if_t< std::is_integral_v<_DataType> && sizeof(_DataType)==1 &&
                                  std::is_integral_v<_NullType> && sizeof(_NullType)==1, bool > = true )
        {
            return ::memset(block, c, n);
        }
        template<typename _DataType, typename _NullType>
        static void* memset_wrap(_DataType *block, const _NullType& c, size_t n,
                std::enable_if_t< !std::is_integral_v<_DataType> || sizeof(_DataType)!=1 ||
                                  !std::is_integral_v<_NullType> || sizeof(_NullType)!=1, bool > = true )
        {
            ABORT("MEMSET shall not be used");
            (void)block;
            (void)c;
            (void)n;
            return nullptr;
        }

        /**
         * clear all elements, zero size
         */
        void clearImpl() noexcept {
            const Size_type size = getSize();
            if( 0 < size ) {
                if( uses_memset ) {
                    memset_wrap(&array[0], nullelem, capacityPlusOne*sizeof(Value_type));
                    readPos  = 0;
                    writePos = 0;
                } else {
                    Size_type localReadPos = readPos;
                    for(Size_type i=0; i<size; i++) {
                        localReadPos = (localReadPos + 1) % capacityPlusOne;
                        array[localReadPos] = nullelem;
                    }
                    if( writePos != localReadPos ) {
                        // Avoid exception, abort!
                        ABORT("copy segment error: this %s, readPos %d/%d; writePos %d", toString().c_str(), readPos.load(), localReadPos, writePos.load());
                    }
                    readPos = localReadPos;
                }
            }
        }

        void cloneFrom(const bool allocArrayAndCapacity, const ringbuffer & source) noexcept {
            if( allocArrayAndCapacity ) {
                capacityPlusOne = source.capacityPlusOne;
                if( nullptr != array ) {
                    freeArray(&array);
                }
                array = newArray(capacityPlusOne);
            } else if( capacityPlusOne != source.capacityPlusOne ) {
                ABORT( ("capacityPlusOne not equal: this "+toString()+", source "+source.toString() ).c_str() );
            }

            readPos = source.readPos.load();
            writePos = source.writePos.load();

            if( use_memcpy ) {
                memcpy(reinterpret_cast<void*>(&array[0]),
                       reinterpret_cast<void*>(const_cast<Value_type*>(&source.array[0])),
                       capacityPlusOne*sizeof(Value_type));
            } else {
                const Size_type size = getSize();
                Size_type localWritePos = readPos;
                for(Size_type i=0; i<size; i++) {
                    localWritePos = (localWritePos + 1) % capacityPlusOne;
                    array[localWritePos] = source.array[localWritePos];
                }
                if( writePos != localWritePos ) {
                    ABORT( ("copy segment error: this "+toString()+", localWritePos "+std::to_string(localWritePos)+"; source "+source.toString()).c_str() );
                }
            }
        }

        void resetImpl(const Value_type * copyFrom, const Size_type copyFromCount) noexcept {
            clearImpl();

            // fill with copyFrom elements
            if( nullptr != copyFrom && 0 < copyFromCount ) {
                if( copyFromCount > capacityPlusOne-1 ) {
                    // new blank resized array
                    if( nullptr != array ) {
                        freeArray(&array);
                    }
                    capacityPlusOne = copyFromCount + 1;
                    array = newArray(capacityPlusOne);
                    readPos  = 0;
                    writePos = 0;
                }
                if( use_memcpy ) {
                    memcpy(reinterpret_cast<void*>(&array[0]),
                           reinterpret_cast<void*>(const_cast<Value_type*>(copyFrom)),
                           copyFromCount*sizeof(Value_type));
                    readPos  = capacityPlusOne - 1; // last read-pos
                    writePos = copyFromCount   - 1; // last write-pos
                } else {
                    Size_type localWritePos = writePos;
                    for(Size_type i=0; i<copyFromCount; i++) {
                        localWritePos = (localWritePos + 1) % capacityPlusOne;
                        array[localWritePos] = copyFrom[i];
                    }
                    writePos = localWritePos;
                }
            }
        }

        Value_type peekImpl(const bool blocking, const int timeoutMS, bool& success) noexcept {
            if( !std::is_copy_constructible_v<Value_type> ) {
                ABORT("Value_type is not copy constructible");
                return nullelem;
            }
            std::unique_lock<std::mutex> lockMultiRead(syncMultiRead); // acquire syncMultiRead, _not_ sync'ing w/ putImpl

            const Size_type oldReadPos = readPos; // SC-DRF acquire atomic readPos, sync'ing with putImpl
            Size_type localReadPos = oldReadPos;
            if( localReadPos == writePos ) {
                if( blocking ) {
                    std::unique_lock<std::mutex> lockWrite(syncWrite); // SC-DRF w/ putImpl via same lock
                    while( localReadPos == writePos ) {
                        if( 0 == timeoutMS ) {
                            cvWrite.wait(lockWrite);
                        } else {
                            std::chrono::steady_clock::time_point t0 = std::chrono::steady_clock::now();
                            std::cv_status s = cvWrite.wait_until(lockWrite, t0 + std::chrono::milliseconds(timeoutMS));
                            if( std::cv_status::timeout == s && localReadPos == writePos ) {
                                success = false;
                                return nullelem;
                            }
                        }
                    }
                } else {
                    success = false;
                    return nullelem;
                }
            }
            localReadPos = (localReadPos + 1) % capacityPlusOne;
            Value_type r = array[localReadPos]; // SC-DRF
            readPos = oldReadPos; // SC-DRF release atomic readPos (complete acquire-release even @ peek)
            success = true;
            return r;
        }

        Value_type moveOutImpl(const bool blocking, const int timeoutMS, bool& success) noexcept {
            std::unique_lock<std::mutex> lockMultiRead(syncMultiRead); // acquire syncMultiRead, _not_ sync'ing w/ putImpl

            const Size_type oldReadPos = readPos; // SC-DRF acquire atomic readPos, sync'ing with putImpl
            Size_type localReadPos = oldReadPos;
            if( localReadPos == writePos ) {
                if( blocking ) {
                    std::unique_lock<std::mutex> lockWrite(syncWrite); // SC-DRF w/ putImpl via same lock
                    while( localReadPos == writePos ) {
                        if( 0 == timeoutMS ) {
                            cvWrite.wait(lockWrite);
                        } else {
                            std::chrono::steady_clock::time_point t0 = std::chrono::steady_clock::now();
                            std::cv_status s = cvWrite.wait_until(lockWrite, t0 + std::chrono::milliseconds(timeoutMS));
                            if( std::cv_status::timeout == s && localReadPos == writePos ) {
                                success = false;
                                return nullelem;
                            }
                        }
                    }
                } else {
                    success = false;
                    return nullelem;
                }
            }
            localReadPos = (localReadPos + 1) % capacityPlusOne;
            Value_type r = std::move( array[localReadPos] ); // SC-DRF
            array[localReadPos] = nullelem;
            {
                std::unique_lock<std::mutex> lockRead(syncRead); // SC-DRF w/ putImpl via same lock
                readPos = localReadPos; // SC-DRF release atomic readPos
                cvRead.notify_all(); // notify waiting putter
            }
            success = true;
            return r;
        }

        bool moveOutImpl(Value_type *dest, const Size_type count, const bool blocking, const int timeoutMS) noexcept {
            std::unique_lock<std::mutex> lockMultiRead(syncMultiRead); // acquire syncMultiRead, _not_ sync'ing w/ putImpl

            Value_type *iter_out = dest;

            if( count >= capacityPlusOne ) {
                return false;
            }

            const Size_type oldReadPos = readPos; // SC-DRF acquire atomic readPos, sync'ing with putImpl
            Size_type localReadPos = oldReadPos;
            Size_type available = getSize();
            if( count > available ) {
                if( blocking ) {
                    std::unique_lock<std::mutex> lockWrite(syncWrite); // SC-DRF w/ putImpl via same lock
                    available = getSize();
                    while( count > available ) {
                        if( 0 == timeoutMS ) {
                            cvWrite.wait(lockWrite);
                            available = getSize();
                        } else {
                            std::chrono::steady_clock::time_point t0 = std::chrono::steady_clock::now();
                            std::cv_status s = cvWrite.wait_until(lockWrite, t0 + std::chrono::milliseconds(timeoutMS));
                            available = getSize();
                            if( std::cv_status::timeout == s && count > available ) {
                                return false;
                            }
                        }
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
            // Since available > 0, we can exclude Empty case.
            Size_type togo_count = count;
            const Size_type localWritePos = writePos;
            if( localReadPos > localWritePos ) {
                // we have a tail
                localReadPos = ( localReadPos + 1 ) % capacityPlusOne; // next-read-pos
                const Size_type tail_count = std::min(togo_count, capacityPlusOne - localReadPos);
                if( use_memcpy ) {
                    memcpy(reinterpret_cast<void*>(iter_out),
                           reinterpret_cast<void*>(&array[localReadPos]),
                           tail_count*sizeof(Value_type));
                    if( uses_memset ) {
                        memset_wrap(&array[localReadPos], nullelem, tail_count*sizeof(Value_type));
                    } else {
                        for(Size_type i=0; i<tail_count; i++) {
                            array[localReadPos+i] = nullelem;
                        }
                    }
                } else {
                    for(Size_type i=0; i<tail_count; i++) {
                        iter_out[i] = std::move( array[localReadPos+i] ); // SC-DRF
                        array[localReadPos+i] = nullelem;
                    }
                }
                localReadPos = ( localReadPos + tail_count - 1 ) % capacityPlusOne; // last read-pos
                togo_count -= tail_count;
                iter_out += tail_count;
            }
            if( togo_count > 0 ) {
                // we have a head
                localReadPos = ( localReadPos + 1 ) % capacityPlusOne; // next-read-pos
                if( use_memcpy ) {
                    memcpy(reinterpret_cast<void*>(iter_out),
                           reinterpret_cast<void*>(&array[localReadPos]),
                           togo_count*sizeof(Value_type));
                    if( uses_memset ) {
                        memset_wrap(&array[localReadPos], nullelem, togo_count*sizeof(Value_type));
                    } else {
                        for(Size_type i=0; i<togo_count; i++) {
                            array[localReadPos+i] = nullelem;
                        }
                    }
                } else {
                    for(Size_type i=0; i<togo_count; i++) {
                        iter_out[i] = array[localReadPos+i];
                        array[localReadPos+i] = nullelem;
                    }
                }
                localReadPos = ( localReadPos + togo_count - 1 ) % capacityPlusOne; // last read-pos
            }
            // Value_type r = std::move( array[localReadPos] ); // SC-DRF
            {
                std::unique_lock<std::mutex> locRead(syncRead); // SC-DRF w/ putImpl via same lock
                readPos = localReadPos; // SC-DRF release atomic readPos
                cvRead.notify_all(); // notify waiting putter
            }
            return true;
        }

        bool dropImpl (const Size_type count, const bool blocking, const int timeoutMS) noexcept {
            std::unique_lock<std::mutex> lockMultiRead(syncMultiRead); // acquire syncMultiRead, _not_ sync'ing w/ putImpl

            if( count >= capacityPlusOne ) {
                return false;
            }

            const Size_type oldReadPos = readPos; // SC-DRF acquire atomic readPos, sync'ing with putImpl
            Size_type localReadPos = oldReadPos;
            Size_type available = getSize();
            if( count > available ) {
                if( blocking ) {
                    std::unique_lock<std::mutex> lockWrite(syncWrite); // SC-DRF w/ putImpl via same lock
                    available = getSize();
                    while( count > available ) {
                        if( 0 == timeoutMS ) {
                            cvWrite.wait(lockWrite);
                            available = getSize();
                        } else {
                            std::chrono::steady_clock::time_point t0 = std::chrono::steady_clock::now();
                            std::cv_status s = cvWrite.wait_until(lockWrite, t0 + std::chrono::milliseconds(timeoutMS));
                            available = getSize();
                            if( std::cv_status::timeout == s && count > available ) {
                                return false;
                            }
                        }
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
            // Since available > 0, we can exclude Empty case.
            Size_type togo_count = count;
            const Size_type localWritePos = writePos;
            if( localReadPos > localWritePos ) {
                // we have a tail
                localReadPos = ( localReadPos + 1 ) % capacityPlusOne; // next-read-pos
                const Size_type tail_count = std::min(togo_count, capacityPlusOne - localReadPos);
                if( uses_memset ) {
                    memset_wrap(&array[localReadPos], nullelem, tail_count*sizeof(Value_type));
                } else {
                    for(Size_type i=0; i<tail_count; i++) {
                        array[localReadPos+i] = nullelem;
                    }
                }
                localReadPos = ( localReadPos + tail_count - 1 ) % capacityPlusOne; // last read-pos
                togo_count -= tail_count;
            }
            if( togo_count > 0 ) {
                // we have a head
                localReadPos = ( localReadPos + 1 ) % capacityPlusOne; // next-read-pos
                if( uses_memset ) {
                    memset_wrap(&array[localReadPos], nullelem, togo_count*sizeof(Value_type));
                } else {
                    for(Size_type i=0; i<togo_count; i++) {
                        array[localReadPos+i] = nullelem;
                    }
                }
                localReadPos = ( localReadPos + togo_count - 1 ) % capacityPlusOne; // last read-pos
            }
            // Value_type r = std::move( array[localReadPos] ); // SC-DRF
            {
                std::unique_lock<std::mutex> lockRead(syncRead); // SC-DRF w/ putImpl via same lock
                readPos = localReadPos; // SC-DRF release atomic readPos
                cvRead.notify_all(); // notify waiting putter
            }
            return true;
        }

        bool moveIntoImpl(Value_type &&e, const bool blocking, const int timeoutMS) noexcept {
            std::unique_lock<std::mutex> lockMultiWrite(syncMultiWrite); // acquire syncMultiWrite, _not_ sync'ing w/ getImpl

            Size_type localWritePos = writePos; // SC-DRF acquire atomic writePos, sync'ing with getImpl
            localWritePos = (localWritePos + 1) % capacityPlusOne;
            if( localWritePos == readPos ) {
                if( blocking ) {
                    std::unique_lock<std::mutex> lockRead(syncRead); // SC-DRF w/ getImpl via same lock
                    while( localWritePos == readPos ) {
                        if( 0 == timeoutMS ) {
                            cvRead.wait(lockRead);
                        } else {
                            std::chrono::steady_clock::time_point t0 = std::chrono::steady_clock::now();
                            std::cv_status s = cvRead.wait_until(lockRead, t0 + std::chrono::milliseconds(timeoutMS));
                            if( std::cv_status::timeout == s && localWritePos == readPos ) {
                                return false;
                            }
                        }
                    }
                } else {
                    return false;
                }
            }
            array[localWritePos] = std::move(e); // SC-DRF
            {
                std::unique_lock<std::mutex> lockWrite(syncWrite); // SC-DRF w/ getImpl via same lock
                writePos = localWritePos; // SC-DRF release atomic writePos
                cvWrite.notify_all(); // notify waiting getter
            }
            return true;
        }

        bool copyIntoImpl(const Value_type &e, const bool blocking, const int timeoutMS) noexcept {
            if( !std::is_copy_constructible_v<Value_type> ) {
                ABORT("Value_type is not copy constructible");
                return false;
            }
            std::unique_lock<std::mutex> lockMultiWrite(syncMultiWrite); // acquire syncMultiWrite, _not_ sync'ing w/ getImpl

            Size_type localWritePos = writePos; // SC-DRF acquire atomic writePos, sync'ing with getImpl
            localWritePos = (localWritePos + 1) % capacityPlusOne;
            if( localWritePos == readPos ) {
                if( blocking ) {
                    std::unique_lock<std::mutex> lockRead(syncRead); // SC-DRF w/ getImpl via same lock
                    while( localWritePos == readPos ) {
                        if( 0 == timeoutMS ) {
                            cvRead.wait(lockRead);
                        } else {
                            std::chrono::steady_clock::time_point t0 = std::chrono::steady_clock::now();
                            std::cv_status s = cvRead.wait_until(lockRead, t0 + std::chrono::milliseconds(timeoutMS));
                            if( std::cv_status::timeout == s && localWritePos == readPos ) {
                                return false;
                            }
                        }
                    }
                } else {
                    return false;
                }
            }
            array[localWritePos] = e; // SC-DRF
            {
                std::unique_lock<std::mutex> lockWrite(syncWrite); // SC-DRF w/ getImpl via same lock
                writePos = localWritePos; // SC-DRF release atomic writePos
                cvWrite.notify_all(); // notify waiting getter
            }
            return true;
        }

        bool copyIntoImpl(const Value_type *first, const Value_type* last, const bool blocking, const int timeoutMS) noexcept {
            if( !std::is_copy_constructible_v<Value_type> ) {
                ABORT("Value_type is not copy constructible");
                return false;
            }
            std::unique_lock<std::mutex> lockMultiWrite(syncMultiWrite); // acquire syncMultiWrite, _not_ sync'ing w/ getImpl

            const Value_type *iter_in = first;
            const Size_type total_count = last - first;

            if( total_count >= capacityPlusOne ) {
                return false;
            }
            Size_type localWritePos = writePos; // SC-DRF acquire atomic writePos, sync'ing with getImpl
            Size_type available = getFreeSlots();
            if( total_count > available ) {
                if( blocking ) {
                    std::unique_lock<std::mutex> lockRead(syncRead); // SC-DRF w/ getImpl via same lock
                    available = getFreeSlots();
                    while( total_count > available ) {
                        if( 0 == timeoutMS ) {
                            cvRead.wait(lockRead);
                            available = getFreeSlots();
                        } else {
                            std::chrono::steady_clock::time_point t0 = std::chrono::steady_clock::now();
                            std::cv_status s = cvRead.wait_until(lockRead, t0 + std::chrono::milliseconds(timeoutMS));
                            available = getFreeSlots();
                            if( std::cv_status::timeout == s && total_count > available ) {
                                return false;
                            }
                        }
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
                if( use_memcpy ) {
                    memcpy(reinterpret_cast<void*>(&array[localWritePos]),
                           reinterpret_cast<void*>(const_cast<Value_type*>(iter_in)),
                           tail_count*sizeof(Value_type));
                } else {
                    for(Size_type i=0; i<tail_count; i++) {
                        array[localWritePos+i] = iter_in[i];
                    }
                }
                localWritePos = ( localWritePos + tail_count - 1 ) % capacityPlusOne; // last write-pos
                togo_count -= tail_count;
                iter_in += tail_count;
            }
            if( togo_count > 0 ) {
                // we have a head
                localWritePos = ( localWritePos + 1 ) % capacityPlusOne; // next-write-pos
                if( use_memcpy ) {
                    memcpy(reinterpret_cast<void*>(&array[localWritePos]),
                           reinterpret_cast<void*>(const_cast<Value_type*>(iter_in)),
                           togo_count*sizeof(Value_type));
                } else {
                    for(Size_type i=0; i<togo_count; i++) {
                        array[localWritePos+i] = iter_in[i];
                    }
                }
                localWritePos = ( localWritePos + togo_count - 1 ) % capacityPlusOne; // last write-pos
            }
            // array[localWritePos] = e; // SC-DRF
            {
                std::unique_lock<std::mutex> lockRead(syncWrite); // SC-DRF w/ getImpl via same lock
                writePos = localWritePos; // SC-DRF release atomic writePos
                cvWrite.notify_all(); // notify waiting getter
            }
            return true;
        }

    public:

        /**
         * Blocks until at least <code>count</code> elements have been put
         * for subsequent get() and getBlocking().
         *
         * @param min_count minimum number of put slots
         * @param timeoutMS
         * @return the number of put elements, available for get() and getBlocking()
         */
        Size_type waitForElements(const Size_type min_count, const int timeoutMS) noexcept {
            std::unique_lock<std::mutex> lockMultiRead(syncMultiRead); // acquire syncMultiRead, _not_ sync'ing w/ putImpl

            Size_type available = getSize();
            if( min_count > available ) {
                std::unique_lock<std::mutex> lockWrite(syncWrite); // SC-DRF w/ putImpl via same lock
                available = getSize();
                while( min_count > available ) {
                    if( 0 == timeoutMS ) {
                        cvWrite.wait(lockWrite);
                        available = getSize();
                    } else {
                        std::chrono::steady_clock::time_point t0 = std::chrono::steady_clock::now();
                        std::cv_status s = cvWrite.wait_until(lockWrite, t0 + std::chrono::milliseconds(timeoutMS));
                        available = getSize();
                        if( std::cv_status::timeout == s && min_count > available ) {
                            return available;
                        }
                    }
                }
            }
            return available;
        }

        /**
         * Blocks until at least <code>count</code> free slots become available
         * for subsequent put() and putBlocking().
         *
         * @param min_count minimum number of free slots
         * @param timeoutMS
         * @return the number of free slots, available for put() and putBlocking()
         */
        Size_type waitForFreeSlots(const Size_type min_count, const int timeoutMS) noexcept {
            std::unique_lock<std::mutex> lockMultiWrite(syncMultiWrite); // acquire syncMultiWrite, _not_ sync'ing w/ getImpl

            Size_type available = getFreeSlots();
            if( min_count > available ) {
                std::unique_lock<std::mutex> lockRead(syncRead); // SC-DRF w/ getImpl via same lock
                available = getFreeSlots();
                while( min_count > available ) {
                    if( 0 == timeoutMS ) {
                        cvRead.wait(lockRead);
                        available = getFreeSlots();
                    } else {
                        std::chrono::steady_clock::time_point t0 = std::chrono::steady_clock::now();
                        std::cv_status s = cvRead.wait_until(lockRead, t0 + std::chrono::milliseconds(timeoutMS));
                        available = getFreeSlots();
                        if( std::cv_status::timeout == s && min_count > available ) {
                            return available;
                        }
                    }
                }
            }
            return available;
        }

        /** Returns a short string representation incl. size/capacity and internal r/w index (impl. dependent). */
        std::string toString() const noexcept {
            const std::string es = isEmpty() ? ", empty" : "";
            const std::string fs = isFull() ? ", full" : "";
            return "ringbuffer<?>[size "+std::to_string(getSize())+" / "+std::to_string(capacityPlusOne-1)+
                    ", writePos "+std::to_string(writePos)+", readPos "+std::to_string(readPos)+es+fs+"]";
        }

        /** Debug functionality - Dumps the contents of the internal array. */
        void dump(FILE *stream, std::string prefix) const noexcept {
#if 0
            fprintf(stream, "%s %s {\n", prefix.c_str(), toString().c_str());
            for(Size_type i=0; i<capacityPlusOne; i++) {
                // fprintf(stream, "\t[%d]: %p\n", i, array[i].get()); // FIXME
            }
            fprintf(stream, "}\n");
#else
            fprintf(stream, "%s %s, array %p\n", prefix.c_str(), toString().c_str(), array);
#endif
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
         * Implementation will allocate an internal array with size of array <code>copyFrom</code> <i>plus one</i>,
         * and copy all elements from array <code>copyFrom</code> into the internal array.
         * </p>
         * @param nullelem The `null` value used to zero removed elements on get*(..) and clear()
         * @param copyFrom mandatory source array determining ring buffer's net {@link #capacity()} and initial content.
         * @throws IllegalArgumentException if <code>copyFrom</code> is <code>nullptr</code>
         */
        ringbuffer(const NullValue_type& nullelem_, const std::vector<Value_type> & copyFrom) noexcept
        : nullelem(nullelem_), capacityPlusOne(copyFrom.size() + 1), array(newArray(capacityPlusOne)),
          readPos(0), writePos(0)
        {
            resetImpl(copyFrom.data(), copyFrom.size());
            _DEBUG_DUMP("ctor(vector<Value_type>)");
        }

        /**
         * @param nullelem The `null` value used to zero removed elements on get*(..) and clear()
         * @param copyFrom
         * @param copyFromSize
         */
        ringbuffer(const NullValue_type& nullelem_, const Value_type * copyFrom, const Size_type copyFromSize) noexcept
        : nullelem(nullelem_), capacityPlusOne(copyFromSize + 1), array(newArray(capacityPlusOne)),
          readPos(0), writePos(0)
        {
            resetImpl(copyFrom, copyFromSize);
            _DEBUG_DUMP("ctor(Value_type*, len)");
        }

        /**
         * Create an empty ring buffer instance w/ the given net <code>capacity</code>.
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
         * Implementation will allocate an internal array of size <code>capacity</code> <i>plus one</i>.
         * </p>
         * @param nullelem The `null` value used to zero removed elements on get*(..) and clear()
         * @param arrayType the array type of the created empty internal array.
         * @param capacity the initial net capacity of the ring buffer
         */
        ringbuffer(const NullValue_type& nullelem_, const Size_type capacity) noexcept
        : nullelem(nullelem_), capacityPlusOne(capacity + 1), array(newArray(capacityPlusOne)),
          readPos(0), writePos(0)
        {
            _DEBUG_DUMP("ctor(capacity)");
        }

        ~ringbuffer() noexcept {
            _DEBUG_DUMP("dtor(def)");
            if( nullptr != array ) {
                freeArray(&array);
            }
        }

        ringbuffer(const ringbuffer &_source) noexcept
        : nullelem(_source.nullelem), capacityPlusOne(_source.capacityPlusOne), array(newArray(capacityPlusOne)),
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
            std::unique_lock<std::mutex> lockMultiReadS(_source.syncMultiRead, std::defer_lock); // utilize std::lock(r, w), allowing mixed order waiting on read/write ops
            std::unique_lock<std::mutex> lockMultiWriteS(_source.syncMultiWrite, std::defer_lock); // otherwise RAII-style relinquish via destructor
            std::unique_lock<std::mutex> lockMultiRead(syncMultiRead, std::defer_lock);          // same for *this instance!
            std::unique_lock<std::mutex> lockMultiWrite(syncMultiWrite, std::defer_lock);
            std::lock(lockMultiReadS, lockMultiWriteS, lockMultiRead, lockMultiWrite);

            if( this == &_source ) {
                return *this;
            }
            nullelem = _source.nullelem;

            if( capacityPlusOne != _source.capacityPlusOne ) {
                cloneFrom(true, _source);
            } else {
                clearImpl(); // clear
                cloneFrom(false, _source);
            }
            _DEBUG_DUMP("assignment(copy.this)");
            _DEBUG_DUMP2(_source, "assignment(copy.source)");
            return *this;
        }

        ringbuffer(ringbuffer &&o) noexcept = default;
        ringbuffer& operator=(ringbuffer &&o) noexcept = default;

        /** Returns the net capacity of this ring buffer. */
        Size_type capacity() const noexcept { return capacityPlusOne-1; }

        /**
         * Releasing all elements by assigning <code>nullelem</code>.
         * <p>
         * {@link #isEmpty()} will return <code>true</code> and
         * {@link #getSize()} will return <code>0</code> after calling this method.
         * </p>
         */
        void clear() noexcept {
            std::unique_lock<std::mutex> lockMultiRead(syncMultiRead, std::defer_lock);          // utilize std::lock(r, w), allowing mixed order waiting on read/write ops
            std::unique_lock<std::mutex> lockMultiWrite(syncMultiWrite, std::defer_lock);        // otherwise RAII-style relinquish via destructor
            std::lock(lockMultiRead, lockMultiWrite);
            clearImpl();
        }

        /**
         * {@link #clear()} all elements and add all <code>copyFrom</code> elements thereafter.
         * @param copyFrom Mandatory array w/ length {@link #capacity()} to be copied into the internal array.
         */
        void reset(const Value_type * copyFrom, const Size_type copyFromCount) noexcept {
            std::unique_lock<std::mutex> lockMultiRead(syncMultiRead, std::defer_lock);          // utilize std::lock(r, w), allowing mixed order waiting on read/write ops
            std::unique_lock<std::mutex> lockMultiWrite(syncMultiWrite, std::defer_lock);        // otherwise RAII-style relinquish via destructor
            std::lock(lockMultiRead, lockMultiWrite);
            resetImpl(copyFrom, copyFromCount);
        }

        void reset(const std::vector<Value_type> & copyFrom) noexcept {
            std::unique_lock<std::mutex> lockMultiRead(syncMultiRead, std::defer_lock);          // utilize std::lock(r, w), allowing mixed order waiting on read/write ops
            std::unique_lock<std::mutex> lockMultiWrite(syncMultiWrite, std::defer_lock);        // otherwise RAII-style relinquish via destructor
            std::lock(lockMultiRead, lockMultiWrite);
            resetImpl(copyFrom.data(), copyFrom.size());
        }

        /** Returns the number of elements in this ring buffer. */
        Size_type getSize() const noexcept {
            const Size_type R = readPos;
            const Size_type W = writePos;
            // W >= R: W - R
            // W <  R: C+1 - R - 1 + W + 1 = C+1 - R + W
            return W >= R ? W - R : capacityPlusOne - R + W;
        }

        /** Returns the number of free slots available to put.  */
        Size_type getFreeSlots() const noexcept { return capacityPlusOne - 1 - getSize(); }

        /** Returns true if this ring buffer is empty, otherwise false. */
        bool isEmpty() const noexcept { return writePos == readPos; /* 0 == size */ }

        /** Returns true if this ring buffer is full, otherwise false. */
        bool isFull() const noexcept { return ( writePos + 1 ) % capacityPlusOne == readPos; /* W == R - 1 */; }

        /**
         * Peeks the next element at the read position w/o modifying pointer, nor blocking.
         * @return <code>nullelem</code> if empty, otherwise the element which would be read next.
         */
        Value_type peek() noexcept {
            bool success;
            return peekImpl(false, 0, success);
        }

        /**
         * Peeks the next element at the read position w/o modifying pointer, nor blocking.
         * @param result storage for the resulting value if successful, otherwise <code>nullelem</code> if empty.
         * @return true if successful, otherwise false.
         */
        bool peek(Value_type& result) noexcept {
            bool success;
            result = peekImpl(false, 0, success);
            return success;
        }

        /**
         * Peeks the next element at the read position w/o modifying pointer, but with blocking.
         * <p>
         * <code>timeoutMS</code> defaults to zero,
         * i.e. infinitive blocking until an element available via put.<br>
         * Otherwise this methods blocks for the given milliseconds.
         * </p>
         * @return <code>nullelem</code> if empty or timeout occurred, otherwise the element which would be read next.
         */
        Value_type peekBlocking(const int timeoutMS=0) noexcept {
            bool success;
            return peekImpl(true, timeoutMS, success);
        }

        /**
         * Peeks the next element at the read position w/o modifying pointer, but with blocking.
         * <p>
         * <code>timeoutMS</code> defaults to zero,
         * i.e. infinitive blocking until an element available via put.<br>
         * Otherwise this methods blocks for the given milliseconds.
         * </p>
         * @param result storage for the resulting value if successful, otherwise <code>nullelem</code> if empty.
         * @return true if successful, otherwise false.
         */
        bool peekBlocking(Value_type& result, const int timeoutMS=0) noexcept {
            bool success;
            result = peekImpl(true, timeoutMS, success);
            return success;
        }

        /**
         * Dequeues the oldest enqueued element if available, otherwise null.
         * <p>
         * The returned ring buffer slot will be set to <code>nullelem</code> to release the reference
         * and move ownership to the caller.
         * </p>
         * <p>
         * Method is non blocking and returns immediately;.
         * </p>
         * @return the oldest put element if available, otherwise <code>nullelem</code>.
         */
        Value_type get() noexcept {
            bool success;
            return moveOutImpl(false, 0, success);
        }

        /**
         * Dequeues the oldest enqueued element if available, otherwise null.
         * <p>
         * The returned ring buffer slot will be set to <code>nullelem</code> to release the reference
         * and move ownership to the caller.
         * </p>
         * <p>
         * Method is non blocking and returns immediately;.
         * </p>
         * @param result storage for the resulting value if successful, otherwise <code>nullelem</code> if empty.
         * @return true if successful, otherwise false.
         */
        bool get(Value_type& result) noexcept {
            bool success;
            result = moveOutImpl(false, 0, success);
            return success;
        }

        /**
         * Dequeues the oldest enqueued element.
         * <p>
         * The returned ring buffer slot will be set to <code>nullelem</code> to release the reference
         * and move ownership to the caller.
         * </p>
         * <p>
         * <code>timeoutMS</code> defaults to zero,
         * i.e. infinitive blocking until an element available via put.<br>
         * Otherwise this methods blocks for the given milliseconds.
         * </p>
         * @return the oldest put element or <code>nullelem</code> if timeout occurred.
         */
        Value_type getBlocking(const int timeoutMS=0) noexcept {
            bool success;
            return moveOutImpl(true, timeoutMS, success);
        }

        /**
         * Dequeues the oldest enqueued element.
         * <p>
         * The returned ring buffer slot will be set to <code>nullelem</code> to release the reference
         * and move ownership to the caller.
         * </p>
         * <p>
         * <code>timeoutMS</code> defaults to zero,
         * i.e. infinitive blocking until an element available via put.<br>
         * Otherwise this methods blocks for the given milliseconds.
         * </p>
         * @param result storage for the resulting value if successful, otherwise <code>nullelem</code> if empty.
         * @return true if successful, otherwise false.
         */
        bool getBlocking(Value_type& result, const int timeoutMS=0) noexcept {
            bool success;
            result = moveOutImpl(true, timeoutMS, success);
            return success;
        }

        /**
         * Dequeues the oldest enqueued count elements by copying them into the given consecutive 'dest' storage.
         * <p>
         * The returned ring buffer slot will be set to <code>nullelem</code> to release the reference
         * and move ownership to the caller.
         * </p>
         * <p>
         * Method is non blocking and returns immediately;.
         * </p>
         * @param dest pointer to first storage element of `count` consecutive elements.
         * @param count number of consecutive elements to get
         * @return true if successful, otherwise false
         */
        bool get(Value_type *dest, const Size_type count) noexcept {
            return moveOutImpl(dest, count, false, 0);
        }

        /**
         * Dequeues the oldest enqueued count elements by copying them into the given consecutive 'dest' storage.
         * <p>
         * The returned ring buffer slot will be set to <code>nullelem</code> to release the reference
         * and move ownership to the caller.
         * </p>
         * <p>
         * <code>timeoutMS</code> defaults to zero,
         * i.e. infinitive blocking until an element available via put.<br>
         * Otherwise this methods blocks for the given milliseconds.
         * </p>
         * @param dest pointer to first storage element of `count` consecutive elements.
         * @param count number of consecutive elements to get
         * @param timeoutMS
         * @return true if successful, otherwise false
         * @return true if successful, otherwise false in case timeout occurred or otherwise.
         */
        bool getBlocking(Value_type *dest, const Size_type count, const int timeoutMS=0) noexcept {
            return moveOutImpl(dest, count, true, timeoutMS);
        }

        /**
         * Drops {@code count} oldest enqueued elements.
         * <p>
         * Method is non blocking and returns immediately;.
         * </p>
         * @param count number of elements to drop from ringbuffer.
         * @return true if successful, otherwise false
         */
        bool drop(const Size_type count) noexcept {
            return dropImpl(count, false, 0);
        }

        /**
         * Drops {@code count} oldest enqueued elements.
         * <p>
         * <code>timeoutMS</code> defaults to zero,
         * i.e. infinitive blocking until an element available via put.<br>
         * Otherwise this methods blocks for the given milliseconds.
         * </p>
         * @param count number of elements to drop from ringbuffer.
         * @return true if successful, otherwise false
         */
        bool dropBlocking(const Size_type count, const int timeoutMS=0) noexcept {
            return dropImpl(count, true, timeoutMS);
        }

        /**
         * Enqueues the given element by moving it into this ringbuffer storage.
         * <p>
         * Returns true if successful, otherwise false in case buffer is full.
         * </p>
         * <p>
         * Method is non blocking and returns immediately;.
         * </p>
         * @return true if successful, otherwise false
         */
        bool put(Value_type && e) noexcept {
            return moveIntoImpl(std::move(e), false, 0);
        }

        /**
         * Enqueues the given element by moving it into this ringbuffer storage.
         * <p>
         * <code>timeoutMS</code> defaults to zero,
         * i.e. infinitive blocking until a free slot becomes available via get.<br>
         * Otherwise this methods blocks for the given milliseconds.
         * </p>
         * @return true if successful, otherwise false in case timeout occurred or otherwise.
         */
        bool putBlocking(Value_type && e, const int timeoutMS=0) noexcept {
            return moveIntoImpl(std::move(e), true, timeoutMS);
        }

        /**
         * Enqueues the given element by copying it into this ringbuffer storage.
         * <p>
         * Returns true if successful, otherwise false in case buffer is full.
         * </p>
         * <p>
         * Method is non blocking and returns immediately;.
         * </p>
         * @return true if successful, otherwise false
         */
        bool put(const Value_type & e) noexcept {
            return copyIntoImpl(e, false, 0);
        }

        /**
         * Enqueues the given element by copying it into this ringbuffer storage.
         * <p>
         * <code>timeoutMS</code> defaults to zero,
         * i.e. infinitive blocking until a free slot becomes available via get.<br>
         * Otherwise this methods blocks for the given milliseconds.
         * </p>
         * @return true if successful, otherwise false in case timeout occurred or otherwise.
         */
        bool putBlocking(const Value_type & e, const int timeoutMS=0) noexcept {
            return copyIntoImpl(e, true, timeoutMS);
        }

        /**
         * Enqueues the given range of consecutive elements by copying it into this ringbuffer storage.
         * <p>
         * Returns true if successful, otherwise false in case buffer is full.
         * </p>
         * <p>
         * Method is non blocking and returns immediately;.
         * </p>
         * @param first pointer to first consecutive element to range of value_type [first, last)
         * @param last pointer to last consecutive element to range of value_type [first, last)
         * @return true if successful, otherwise false
         */
        bool put(const Value_type *first, const Value_type* last) noexcept {
            return copyIntoImpl(first, last, false, 0);
        }

        /**
         * Enqueues the given range of consecutive elementa by copying it into this ringbuffer storage.
         * <p>
         * <code>timeoutMS</code> defaults to zero,
         * i.e. infinitive blocking until a free slot becomes available via get.<br>
         * Otherwise this methods blocks for the given milliseconds.
         * </p>
         * @param first pointer to first consecutive element to range of value_type [first, last)
         * @param last pointer to last consecutive element to range of value_type [first, last)
         * @param timeoutMS
         * @return true if successful, otherwise false in case timeout occurred or otherwise.
         */
        bool putBlocking(const Value_type *first, const Value_type* last, const int timeoutMS=0) noexcept {
            return copyIntoImpl(first, last, true, timeoutMS);
        }

        /**
         * Resizes this ring buffer's capacity.
         * <p>
         * New capacity must be greater than current size.
         * </p>
         */
        void recapacity(const Size_type newCapacity) {
            std::unique_lock<std::mutex> lockMultiRead(syncMultiRead, std::defer_lock);          // utilize std::lock(r, w), allowing mixed order waiting on read/write ops
            std::unique_lock<std::mutex> lockMultiWrite(syncMultiWrite, std::defer_lock);        // otherwise RAII-style relinquish via destructor
            std::lock(lockMultiRead, lockMultiWrite);
            const Size_type size = getSize();

            if( capacityPlusOne == newCapacity+1 ) {
                return;
            }
            if( size > newCapacity ) {
                throw IllegalArgumentException("amount "+std::to_string(newCapacity)+" < size, "+toString(), E_FILE_LINE);
            }

            // save current data
            Size_type oldCapacityPlusOne = capacityPlusOne;
            Value_type * oldArray = array;
            Size_type oldReadPos = readPos;

            // new blank resized array
            capacityPlusOne = newCapacity + 1;
            array = newArray(capacityPlusOne);
            readPos = 0;
            writePos = 0;

            // copy saved data
            if( nullptr != oldArray && 0 < size ) {
                Size_type localWritePos = writePos;
                for(Size_type i=0; i<size; i++) {
                    localWritePos = (localWritePos + 1) % capacityPlusOne;
                    oldReadPos = (oldReadPos + 1) % oldCapacityPlusOne;
                    array[localWritePos] = std::move( oldArray[oldReadPos] );
                }
                writePos = localWritePos;
            }
            freeArray(&oldArray); // and release
        }
};

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
