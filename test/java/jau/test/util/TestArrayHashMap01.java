/**
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2021 Gothel Software e.K.
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

package jau.test.util;

import java.io.IOException;
import java.util.List;
import java.util.Map;

import org.jau.util.ArrayHashMap;
import org.jau.util.ArrayHashSet;
import org.junit.Assert;
import org.junit.FixMethodOrder;
import org.junit.Test;
import org.junit.runners.MethodSorters;

import jau.test.junit.util.JunitTracer;

@FixMethodOrder(MethodSorters.NAME_ASCENDING)
public class TestArrayHashMap01 extends JunitTracer {

    public static class Dummy {
        int i1, i2, i3;

        public Dummy(final int i1, final int i2, final int i3) {
            this.i1 = i1;
            this.i2 = i2;
            this.i3 = i3;
        }

        @Override
        public boolean equals(final Object o) {
            if(o instanceof Dummy) {
                final Dummy d = (Dummy)o;
                return this.i1 == d.i1 &&
                       this.i2 == d.i2 &&
                       this.i3 == d.i3 ;
            }
            return false;
        }

        @Override
        public final int hashCode() {
            // 31 * x == (x << 5) - x
            int hash = 31 + i1;
            hash = ((hash << 5) - hash) + i2;
            hash = ((hash << 5) - hash) + i3;
            return hash;
        }

        @Override
        public String toString() {
            return "Dummy["+super.toString()+": "+i1+", "+i2+", "+i3+"]";
        }
    }

    void populate(final Map<Integer, Dummy> l, final int start, final int len,
                  final int i2, final int i3, final int expectedPlusSize) {
        final int oldSize = l.size();
        for(int pos = start+len-1; pos>=start; pos--) {
            l.put(pos, new Dummy(pos, i2, i3));
        }
        Assert.assertEquals(expectedPlusSize, l.size() - oldSize);
    }
    boolean checkOrder(final List<Dummy> l, final int startIdx, final int start, final int len) {
        for(int i=0; i<len; i++) {
            final Dummy d = l.get(startIdx+i);
            final int i1 = start+len-1-i;
            if( d.i1 != i1 ) {
                return false;
            }
        }
        return true;
    }

    @Test
    public void test01ArrayHashMapWithNullValue() {
        testArrayHashMapImpl(true);
    }
    @Test
    public void test02ArrayHashSetWithoutNullValue() {
        testArrayHashMapImpl(false);
    }
    void testArrayHashMapImpl(final boolean supportNullValue) {
        final ArrayHashMap<Integer, Dummy> l =
                new ArrayHashMap<Integer, Dummy>(supportNullValue,
                                        ArrayHashSet.DEFAULT_INITIAL_CAPACITY,
                                        ArrayHashSet.DEFAULT_LOAD_FACTOR);
        Assert.assertEquals(supportNullValue, l.supportsNullValue());
        final int p7_22_34_key, p7_22_34_idx;
        final Dummy p7_22_34_orig;
        final int p6_22_34_key, p6_22_34_idx;
        final Dummy p6_22_34_orig;
        {
            populate(l, 10, 100, 22, 34, 100); // [109 .. 10]
            Assert.assertTrue(checkOrder(l.getData(), 0, 10, 100));
            populate(l, 10, 100, 22, 34,   0); // [109 .. 10]
            Assert.assertTrue(checkOrder(l.getData(), 0, 10, 100));
            populate(l,  6,   5, 22, 34,   4); // [  9 ..  6], 10 already exists
            Assert.assertTrue(checkOrder(l.getData(), 100, 6, 4));
            p7_22_34_idx = l.size() - 2;
            p7_22_34_key = 7;
            p7_22_34_orig = l.get(p7_22_34_key);
            p6_22_34_idx = l.size() - 1;
            p6_22_34_key = 6;
            p6_22_34_orig = l.get(p6_22_34_key);
        }
        Assert.assertNotNull(p7_22_34_orig);
        Assert.assertEquals(7, p7_22_34_orig.i1);
        Assert.assertEquals(l.getData().get(p7_22_34_idx), p7_22_34_orig);
        Assert.assertNotNull(p6_22_34_orig);
        Assert.assertEquals(6, p6_22_34_orig.i1);
        Assert.assertEquals(l.getData().get(p6_22_34_idx), p6_22_34_orig);

        final Dummy p7_22_34_other = new Dummy(7, 22, 34);
        Assert.assertEquals(p7_22_34_other, p7_22_34_orig);
        Assert.assertTrue(p7_22_34_other.hashCode() == p7_22_34_orig.hashCode());
        Assert.assertTrue(p7_22_34_other != p7_22_34_orig); // diff reference
        final Dummy p6_22_34_other = new Dummy(6, 22, 34);
        Assert.assertEquals(p6_22_34_other, p6_22_34_orig);
        Assert.assertTrue(p6_22_34_other.hashCode() == p6_22_34_orig.hashCode());
        Assert.assertTrue(p6_22_34_other != p6_22_34_orig); // diff reference

        // fast identity ..
        Dummy q = l.get(p6_22_34_key);
        Assert.assertNotNull(q);
        Assert.assertEquals(p6_22_34_other, q);
        Assert.assertTrue(p6_22_34_other.hashCode() == q.hashCode());
        Assert.assertTrue(p6_22_34_other != q); // diff reference
        Assert.assertTrue(p6_22_34_orig == q); // same reference

        Assert.assertTrue(l.containsValue(q));
        Assert.assertTrue(l.containsValue(p6_22_34_other)); // add equivalent

        q = l.put(p6_22_34_key, p6_22_34_other); // override w/ diff hash-obj
        Assert.assertNotNull(q);
        Assert.assertEquals(p6_22_34_other, q);
        Assert.assertTrue(p6_22_34_other.hashCode() == q.hashCode());
        Assert.assertTrue(p6_22_34_other != q); // diff reference new != old (q)
        Assert.assertTrue(p6_22_34_orig == q); // same reference orig == old (q)
        Assert.assertTrue(checkOrder(l.getData(), 0, 10, 100));
        Assert.assertTrue(checkOrder(l.getData(), 100, 6, 4));

        final Dummy p1_2_3 = new Dummy(1, 2, 3); // a new one ..
        q = l.put(1, p1_2_3); // added test
        Assert.assertNull(q);

        final Dummy pNull = null;
        NullPointerException npe = null;
        try {
            q = l.put(0, pNull);
            Assert.assertNull(q);
        } catch (final NullPointerException _npe) { npe = _npe; }
        if( l.supportsNullValue() ) {
            Assert.assertNull(npe);
        } else {
            Assert.assertNotNull(npe);
        }
    }

    public static void main(final String args[]) throws IOException {
        final String tstname = TestArrayHashMap01.class.getName();
        org.junit.runner.JUnitCore.main(tstname);
    }

}
