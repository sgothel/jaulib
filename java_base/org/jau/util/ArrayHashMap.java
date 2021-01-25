/**
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2020 Gothel Software e.K.
 * Copyright (c) 2015 Gothel Software e.K.
 * Copyright (c) 2015 JogAmp Community.
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

package org.jau.util;

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.Set;

/**
 * {@link HashMap} implementation backed by an {@link ArrayList} to preserve order of values.
 *
 * Implementation properties are:
 *  <ul>
 *  <li> Unique elements utilizing {@link java.lang.Object#hashCode()} for O(1) operations, see below.</li>
 *  <li> Java 1.5 compatible</li>
 *  </ul>
 *
 * O(1) operations:
 * <ul>
 * <li> put new key-value-pair(s) </li>
 * <li> test for containment </li>
 * <li> trying to remove non existent elements </li>
 * </ul>
 *
 * O(n) operations:
 * <ul>
 * <li> put existing key-value-pair(s) </li>
 * <li> removing existing elements</li>
 * </ul>
 *
 * For thread safety, the application shall decorate access to instances via
 * {@link org.jau.util.parallel.locks.RecursiveLock}.
 *
*/
public class ArrayHashMap<K, V>
    implements Cloneable, Map<K, V>
{
    /**
     * Default load factor: {@value}
     */
    public static final float DEFAULT_LOAD_FACTOR = 0.75f;
    /**
     * The default initial capacity: {@value}
     */
    public static final int DEFAULT_INITIAL_CAPACITY = 16;

    private final HashMap<K,V> map; // key -> object
    private final ArrayList<V> data; // list of objects
    private final boolean supportNullValue;

    /**
     *
     * @param supportNullValue Use {@code true} for default behavior, i.e. {@code null} can be a valid value.
     *                         Use {@code false} if {@code null} is not a valid value,
     *                         here {@link #put(Object, Object)} and {@link #remove(Object)} will be optimized.
     * @param initialCapacity use {@link #DEFAULT_INITIAL_CAPACITY} for default
     * @param loadFactor use {@link #DEFAULT_LOAD_FACTOR} for default
     * @see #supportsNullValue()
     */
    public ArrayHashMap(final boolean supportNullValue, final int initialCapacity, final float loadFactor) {
        this.map  = new HashMap<K,V>(initialCapacity, loadFactor);
        this.data = new ArrayList<V>(initialCapacity);
        this.supportNullValue = supportNullValue;
    }

    /**
     * @return a shallow copy of this ArrayHashMap, elements are not copied.
     */
    public ArrayHashMap(final ArrayHashMap<K, V> o) {
        map = new HashMap<K, V>(o.map);
        data = new ArrayList<V>(o.data);
        supportNullValue = o.supportNullValue;
    }

    /**
     * Returns {@code true} for default behavior, i.e. {@code null} can be a valid value.
     * <p>
     * Returns {@code false} if {@code null} is not a valid value,
     * here {@link #put(Object, Object)} and {@link #remove(Object)} are optimized operations.
     * </p>
     * @see #ArrayHashMap(boolean, int, float)
     */
    public final boolean supportsNullValue() { return supportNullValue; }

    //
    // Cloneable
    //

    /**
     * Implementation uses {@link #ArrayHashMap(ArrayHashMap)}.
     * @return a shallow copy of this ArrayHashMap, elements are not copied.
     */
    @Override
    public final Object clone() {
        return new ArrayHashMap<K, V>(this);
    }

    /**
     * Returns this object ordered ArrayList. Use w/ care, it's not a copy.
     * @see #toArrayList()
     */
    public final ArrayList<V> getData() { return data; }

    /**
     * @return a shallow copy of this ArrayHashMap's ArrayList, elements are not copied.
     * @see #getData()
     */
    public final ArrayList<V> toArrayList() {
        return new ArrayList<V>(data);
    }

    /** Returns this object hash map. Use w/ care, it's not a copy. */
    public final HashMap<K,V> getMap() { return map; }

    @Override
    public final String toString() { return data.toString(); }

    //
    // Map
    //

    @Override
    public final void clear() {
        data.clear();
        map.clear();
    }

    @Override
    public Set<K> keySet() {
        return map.keySet();
    }

    /**
     * {@inheritDoc}
     * <p>
     * See {@link #getData()} and {@link #toArrayList()}.
     * </p>
     * @see #getData()
     * @see #toArrayList()
     */
    @Override
    public Collection<V> values() {
        return map.values();
    }

    @Override
    public Set<java.util.Map.Entry<K, V>> entrySet() {
        return map.entrySet();
    }

    @Override
    public final V get(final Object key) {
        return map.get(key);
    }

    /**
     * {@inheritDoc}
     * <p>
     * This is an O(1) operation, in case the key does not exist,
     * otherwise O(n).
     * </p>
     * @throws NullPointerException if {@code value} is {@code null} but {@link #supportsNullValue()} == {@code false}
     */
    @Override
    public final V put(final K key, final V value) throws NullPointerException {
        final V oldValue;
        if( supportNullValue ) {
            // slow path
            final boolean exists = map.containsKey(key);
            if(!exists) {
                // !exists
                if( null != ( oldValue = map.put(key, value) ) ) {
                    // slips a valid null ..
                    throw new InternalError("Already existing, but checked before: "+key+" -> "+oldValue);
                }
            } else {
                // exists
                oldValue = map.put(key, value);
                if( !data.remove(oldValue) ) {
                    throw new InternalError("Already existing, but not in list: "+oldValue);
                }
            }
        } else {
            checkNullValue(value);
            // fast path
            if( null != ( oldValue = map.put(key, value) ) ) {
                // exists
                if( !data.remove(oldValue) ) {
                    throw new InternalError("Already existing, but not in list: "+oldValue);
                }
            }
        }
        if(!data.add(value)) {
            throw new InternalError("Couldn't add value to list: "+value);
        }
        return oldValue;
    }

    @Override
    public void putAll(final Map<? extends K, ? extends V> m) {
        for (final Iterator<? extends Map.Entry<? extends K, ? extends V>> i = m.entrySet().iterator(); i.hasNext(); ) {
            final Map.Entry<? extends K, ? extends V> e = i.next();
            put(e.getKey(), e.getValue());
        }
    }

    /**
     * {@inheritDoc}
     * <p>
     * This is an O(1) operation, in case the key does not exist,
     * otherwise O(n).
     * </p>
     */
    @Override
    public final V remove(final Object key) {
        if( supportNullValue ) {
            if( map.containsKey(key) ) {
                // exists
                final V oldValue = map.remove(key);
                if ( !data.remove(oldValue) ) {
                    throw new InternalError("Couldn't remove prev mapped pair: "+key+" -> "+oldValue);
                }
                return oldValue;
            }
        } else {
            final V oldValue;
            if ( null != (oldValue = map.remove(key) ) ) {
                // exists
                if ( !data.remove(oldValue) ) {
                    throw new InternalError("Couldn't remove prev mapped pair: "+key+" -> "+oldValue);
                }
            }
            return oldValue;
        }
        return null;
    }

    @Override
    public final boolean containsKey(final Object key) {
        return map.containsKey(key);
    }

    @Override
    public boolean containsValue(final Object value) {
        return map.containsValue(value);
    }

    @Override
    public final boolean equals(final Object arrayHashMap) {
        if ( !(arrayHashMap instanceof ArrayHashMap) ) {
            return false;
        }
        return map.equals(((ArrayHashMap<?,?>)arrayHashMap).map);
    }

    @Override
    public final int hashCode() {
        return map.hashCode();
    }

    @Override
    public final boolean isEmpty() {
        return data.isEmpty();
    }

    @Override
    public final int size() {
        return data.size();
    }

    private static final void checkNullValue(final Object value) throws NullPointerException {
        if( null == value ) {
            throw new NullPointerException("Null value not supported");
        }
    }
}
