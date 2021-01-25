/**
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2021 Gothel Software e.K.
 * Copyright (c) 2013 Gothel Software e.K.
 * Copyright (c) 2013 JogAmp Community.
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


import org.junit.Assert;
import org.junit.Test;

public abstract class RingBuffer01Base {
    private static boolean DEBUG = false;

    public abstract Ringbuffer<Integer> createEmpty(int initialCapacity);
    public abstract Ringbuffer<Integer> createFull(Integer[] source);

    public Integer[] createIntArray(final int capacity, final int startValue) {
        final Integer[] array = new Integer[capacity];
        for(int i=0; i<capacity; i++) {
            array[i] = Integer.valueOf(startValue+i);
        }
        return array;
    }

    private void readTestImpl(final Ringbuffer<Integer> rb, final boolean clearRef, final int capacity, final int len, final int startValue) {
        final int preSize = rb.size();
        Assert.assertEquals("Wrong capacity "+rb, capacity, rb.capacity());
        Assert.assertTrue("Too low capacity to read "+len+" elems: "+rb, capacity-len >= 0);
        Assert.assertTrue("Too low size to read "+len+" elems: "+rb, preSize >= len);
        Assert.assertTrue("Is empty "+rb, !rb.isEmpty());

        for(int i=0; i<len; i++) {
            final Integer vI = rb.get();
            Assert.assertNotNull("Empty at read #"+(i+1)+": "+rb, vI);
            Assert.assertEquals("Wrong value at read #"+(i+1)+": "+rb, startValue+i, vI.intValue());
        }

        Assert.assertEquals("Invalid size "+rb, preSize-len, rb.size());
        Assert.assertTrue("Invalid free slots after reading "+len+": "+rb, rb.getFreeSlots()>= len);
        Assert.assertTrue("Is full "+rb, !rb.isFull());
    }

    private void writeTestImpl(final Ringbuffer<Integer> rb, final int capacity, final int len, final int startValue) {
        final int preSize = rb.size();

        Assert.assertEquals("Wrong capacity "+rb, capacity, rb.capacity());
        Assert.assertTrue("Too low capacity to write "+len+" elems: "+rb, capacity-len >= 0);
        Assert.assertTrue("Too low size to write "+len+" elems: "+rb, preSize+len <= capacity);
        Assert.assertTrue("Is full "+rb, !rb.isFull());

        for(int i=0; i<len; i++) {
            Assert.assertTrue("Buffer is full at put #"+i+": "+rb, rb.put( Integer.valueOf(startValue+i) ));
        }

        Assert.assertEquals("Invalid size "+rb, preSize+len, rb.size());
        Assert.assertTrue("Is empty "+rb, !rb.isEmpty());
    }

    private void moveGetPutImpl(final Ringbuffer<Integer> rb, final int pos) {
        Assert.assertTrue("RB is empty "+rb, !rb.isEmpty());
        for(int i=0; i<pos; i++) {
            Assert.assertEquals("MoveFull.get failed "+rb, i, rb.get().intValue());
            Assert.assertTrue("MoveFull.put failed "+rb, rb.put(i));
        }
    }

    private void movePutGetImpl(final Ringbuffer<Integer> rb, final int pos) {
        Assert.assertTrue("RB is full "+rb, !rb.isFull());
        for(int i=0; i<pos; i++) {
            Assert.assertTrue("MoveEmpty.put failed "+rb, rb.put(600+i));
            Assert.assertEquals("MoveEmpty.get failed "+rb, 600+i, rb.get().intValue());
        }
    }

    @Test
    public void test01_FullRead() {
        final int capacity = 11;
        final Integer[] source = createIntArray(capacity, 0);
        final Ringbuffer<Integer> rb = createFull(source);
        Assert.assertEquals("Not full size "+rb, capacity, rb.size());
        Assert.assertTrue("Not full "+rb, rb.isFull());

        readTestImpl(rb, true, capacity, capacity, 0);
        Assert.assertTrue("Not empty "+rb, rb.isEmpty());
    }

    @Test
    public void test02_EmptyWrite() {
        final int capacity = 11;
        final Ringbuffer<Integer> rb = createEmpty(capacity);
        Assert.assertEquals("Not zero size "+rb, 0, rb.size());
        Assert.assertTrue("Not empty "+rb, rb.isEmpty());

        writeTestImpl(rb, capacity, capacity, 0);
        Assert.assertEquals("Not full size "+rb, capacity, rb.size());
        Assert.assertTrue("Not full "+rb, rb.isFull());

        readTestImpl(rb, true, capacity, capacity, 0);
        Assert.assertTrue("Not empty "+rb, rb.isEmpty());
    }

    @Test
    public void test03_FullReadReset() {
        final int capacity = 11;
        final Integer[] source = createIntArray(capacity, 0);
        final Ringbuffer<Integer> rb = createFull(source);
        Assert.assertTrue("Not full "+rb, rb.isFull());

        rb.resetFull(source);
        Assert.assertTrue("Not full "+rb, rb.isFull());

        readTestImpl(rb, false, capacity, capacity, 0);
        Assert.assertTrue("Not empty "+rb, rb.isEmpty());

        rb.resetFull(source);
        Assert.assertTrue("Not full "+rb, rb.isFull());

        readTestImpl(rb, false, capacity, capacity, 0);
        Assert.assertTrue("Not empty "+rb, rb.isEmpty());
    }

    @Test
    public void test04_EmptyWriteClear() {
        final int capacity = 11;
        final Ringbuffer<Integer> rb = createEmpty(capacity);
        Assert.assertTrue("Not empty "+rb, rb.isEmpty());

        rb.clear();
        Assert.assertTrue("Not empty "+rb, rb.isEmpty());

        writeTestImpl(rb, capacity, capacity, 0);
        Assert.assertTrue("Not full "+rb, rb.isFull());

        readTestImpl(rb, false, capacity, capacity, 0);
        Assert.assertTrue("Not empty "+rb, rb.isEmpty());

        rb.clear();
        Assert.assertTrue("Not empty "+rb, rb.isEmpty());

        writeTestImpl(rb, capacity, capacity, 0);
        Assert.assertTrue("Not full "+rb, rb.isFull());

        readTestImpl(rb, false, capacity, capacity, 0);
        Assert.assertTrue("Not empty "+rb, rb.isEmpty());
    }

    @Test
    public void test05_ReadResetMid01() {
        final int capacity = 11;
        final Integer[] source = createIntArray(capacity, 0);
        final Ringbuffer<Integer> rb = createFull(source);
        Assert.assertTrue("Not full "+rb, rb.isFull());

        rb.resetFull(source);
        Assert.assertTrue("Not full "+rb, rb.isFull());

        readTestImpl(rb, false, capacity, 5, 0);
        Assert.assertTrue("Is empty "+rb, !rb.isEmpty());
        Assert.assertTrue("Is Full "+rb, !rb.isFull());

        if( DEBUG ) {
            rb.dump(System.err, "ReadReset01["+5+"].pre0");
        }
        rb.resetFull(source);
        Assert.assertTrue("Not full "+rb, rb.isFull());
        if( DEBUG ) {
            rb.dump(System.err, "ReadReset01["+5+"].post");
        }

        readTestImpl(rb, false, capacity, capacity, 0);
        Assert.assertTrue("Not empty "+rb, rb.isEmpty());
    }

    @Test
    public void test06_ReadResetMid02() {
        final int capacity = 11;
        final Integer[] source = createIntArray(capacity, 0);
        final Ringbuffer<Integer> rb = createFull(source);
        Assert.assertTrue("Not full "+rb, rb.isFull());

        rb.resetFull(source);
        Assert.assertTrue("Not full "+rb, rb.isFull());

        moveGetPutImpl(rb, 5);
        // readTestImpl(rb, false, capacity, 5, 0);
        // Assert.assertTrue("Is empty "+rb, !rb.isEmpty());
        // Assert.assertTrue("Is Full "+rb, !rb.isFull());

        if( DEBUG ) {
            rb.dump(System.err, "ReadReset02["+5+"].pre0");
        }
        rb.resetFull(source);
        Assert.assertTrue("Not full "+rb, rb.isFull());
        if( DEBUG ) {
            rb.dump(System.err, "ReadReset02["+5+"].post");
        }

        readTestImpl(rb, false, capacity, capacity, 0);
        Assert.assertTrue("Not empty "+rb, rb.isEmpty());
    }

    private void test_GrowEmptyImpl(final int initCapacity, final int pos) {
        final int growAmount = 5;
        final int grownCapacity = initCapacity+growAmount;
        final Integer[] growArray = new Integer[growAmount];
        for(int i=0; i<growAmount; i++) {
            growArray[i] = Integer.valueOf(100+i);
        }
        final Ringbuffer<Integer> rb = createEmpty(initCapacity);

        if( DEBUG ) {
            rb.dump(System.err, "GrowEmpty["+pos+"].pre0");
        }
        movePutGetImpl(rb, pos);
        if( DEBUG ) {
            rb.dump(System.err, "GrowEmpty["+pos+"].pre1");
        }
        rb.growEmptyBuffer(growArray);
        if( DEBUG ) {
            rb.dump(System.err, "GrowEmpty["+pos+"].post");
        }

        Assert.assertEquals("Wrong capacity "+rb, grownCapacity, rb.capacity());
        Assert.assertEquals("Not growAmount size "+rb, growAmount, rb.size());
        Assert.assertTrue("Is full "+rb, !rb.isFull());
        Assert.assertTrue("Is empty "+rb, !rb.isEmpty());

        for(int i=0; i<growAmount; i++) {
            final Integer vI = rb.get();
            Assert.assertNotNull("Empty at read #"+(i+1)+": "+rb, vI);
            Assert.assertEquals("Wrong value at read #"+(i+1)+": "+rb, 100+i, vI.intValue());
        }

        Assert.assertEquals("Not zero size "+rb, 0, rb.size());
        Assert.assertTrue("Not empty "+rb, rb.isEmpty());
        Assert.assertTrue("Is full "+rb, !rb.isFull());
    }
    @Test
    public void test10_GrowEmpty01_Begin() {
        test_GrowEmptyImpl(11, 0);
    }
    @Test
    public void test11_GrowEmpty02_Begin2() {
        test_GrowEmptyImpl(11, 0+2);
    }
    @Test
    public void test12_GrowEmpty03_End() {
        test_GrowEmptyImpl(11, 11-1);
    }
    @Test
    public void test13_GrowEmpty04_End2() {
        test_GrowEmptyImpl(11, 11-1-2);
    }

    private void test_GrowFullImpl(final int initCapacity, final int pos, final boolean debug) {
        final int growAmount = 5;
        final int grownCapacity = initCapacity+growAmount;
        final Integer[] source = createIntArray(initCapacity, 0);
        final Ringbuffer<Integer> rb = createFull(source);

        if( DEBUG || debug ) {
            rb.dump(System.err, "GrowFull["+pos+"].pre0");
        }
        moveGetPutImpl(rb, pos);
        if( DEBUG || debug ) {
            rb.dump(System.err, "GrowFull["+pos+"].pre1");
        }
        rb.growFullBuffer(growAmount);
        if( DEBUG || debug ) {
            rb.dump(System.err, "GrowFull["+pos+"].post");
        }

        Assert.assertEquals("Wrong capacity "+rb, grownCapacity, rb.capacity());
        Assert.assertEquals("Not orig size "+rb, initCapacity, rb.size());
        Assert.assertTrue("Is full "+rb, !rb.isFull());
        Assert.assertTrue("Is empty "+rb, !rb.isEmpty());

        for(int i=0; i<growAmount; i++) {
            Assert.assertTrue("Buffer is full at put #"+i+": "+rb, rb.put( Integer.valueOf(100+i) ));
        }
        Assert.assertEquals("Not new size "+rb, grownCapacity, rb.size());
        Assert.assertTrue("Not full "+rb, rb.isFull());

        for(int i=0; i<initCapacity; i++) {
            final Integer vI = rb.get();
            Assert.assertNotNull("Empty at read #"+(i+1)+": "+rb, vI);
            Assert.assertEquals("Wrong value at read #"+(i+1)+": "+rb, (pos+i)%initCapacity, vI.intValue());
        }
        for(int i=0; i<growAmount; i++) {
            final Integer vI = rb.get();
            Assert.assertNotNull("Empty at read #"+(i+1)+": "+rb, vI);
            Assert.assertEquals("Wrong value at read #"+(i+1)+": "+rb, 100+i, vI.intValue());
        }

        Assert.assertEquals("Not zero size "+rb, 0, rb.size());
        Assert.assertTrue("Not empty "+rb, rb.isEmpty());
        Assert.assertTrue("Is full "+rb, !rb.isFull());
    }
    @Test
    public void test20_GrowFull01_Begin() {
        test_GrowFullImpl(11, 0, false);
    }
    @Test
    public void test21_GrowFull02_Begin1() {
        test_GrowFullImpl(11, 0+1, false);
    }
    @Test
    public void test22_GrowFull03_Begin2() {
        test_GrowFullImpl(11, 0+2, false);
    }
    @Test
    public void test23_GrowFull04_Begin3() {
        test_GrowFullImpl(11, 0+3, false);
    }
    @Test
    public void test24_GrowFull05_End() {
        test_GrowFullImpl(11, 11-1, false);
    }
    @Test
    public void test25_GrowFull11_End1() {
        test_GrowFullImpl(11, 11-1-1, false);
    }
    @Test
    public void test26_GrowFull12_End2() {
        test_GrowFullImpl(11, 11-1-2, false);
    }
    @Test
    public void test27_GrowFull13_End3() {
        test_GrowFullImpl(11, 11-1-3, false);
    }
}
