package org.strobe.commons.lang;

import org.junit.jupiter.api.Assertions;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;

import java.util.Iterator;

public class UnboundedArrayTest {

    private UnboundedArray<String> array;

    @BeforeEach
    void beforeEach(){
        array = new UnboundedArray<>(8);
    }

    @Test
    void arraySetTest() {
        array.set(0, "abc");
        Assertions.assertEquals(array.get(0), "abc");
        array.set(8, "xyz");
        Assertions.assertEquals(array.get(8), "xyz");
        array.set(16, "cde");
        Assertions.assertEquals(array.get(16), "cde");
    }

    @Test
    void  arrayAddTest(){
        array.add("abc");
        array.add("xyz");
        array.set(7, "str");
        array.add("resize");
        Assertions.assertEquals(array.get(0), "abc");
        Assertions.assertEquals(array.get(1), "xyz");
        Assertions.assertNull(array.get(3));
        Assertions.assertNull(array.get(6));
        Assertions.assertEquals(array.get(7), "str");
        Assertions.assertEquals(array.get(2), "resize");
        //assert to error
        array.add("a");
        array.add("b");
        array.add("c");
        array.add("d");
        array.add("e");
    }

    @Test
    void arrayThrowsTest(){
        Assertions.assertThrows(IndexOutOfBoundsException.class, ()->array.get(-1));
        Assertions.assertThrows(IndexOutOfBoundsException.class, ()->array.set(-1, "str"));
        Assertions.assertThrows(IllegalArgumentException.class, ()->array.add(null));
    }

    @Test
    void arrayIteratorTest(){
        array.add("abc");
        array.set(3, "xyz");
        array.set(4, "foo");
        Iterator<String> iterator = array.iterator();
        Assertions.assertTrue(iterator.hasNext());
        Assertions.assertEquals(iterator.next(), "abc");
        Assertions.assertTrue(iterator.hasNext());
        Assertions.assertEquals(iterator.next(), "xyz");
        Assertions.assertTrue(iterator.hasNext());
        Assertions.assertEquals(iterator.next(), "foo");
        Assertions.assertFalse(iterator.hasNext());
    }

    @Test
    void arrayIsEmptyTest(){
        Assertions.assertTrue(array.isEmpty());
        array.add("abc");
        Assertions.assertFalse(array.isEmpty());
    }

}
