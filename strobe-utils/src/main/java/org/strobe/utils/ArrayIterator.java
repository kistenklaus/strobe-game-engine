package org.strobe.utils;

import java.util.Iterator;

public class ArrayIterator<T> implements Iterator<T> {

    private final boolean iterateNull;
    private final T[] array;
    private int next = 0;

    public ArrayIterator(T[] array, boolean iterateNull) {
        this.array = array;
        this.iterateNull = iterateNull;
    }

    public ArrayIterator(T[] array) {
        this(array, true);
    }

    @Override
    public boolean hasNext() {
        if (iterateNull) {
            return next < array.length;
        } else {
            if (next >= array.length) return false;
            while (array[next] == null) {
                next++;
                if(next >= array.length)return false;
            }
            return true;
        }
    }

    @Override
    public T next() {
        return array[next++];
    }
}
