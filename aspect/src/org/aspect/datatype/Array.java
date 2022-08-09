package org.aspect.datatype;

import java.util.Iterator;

public class Array<T> {

    private T[] data;
    private int size;

    public Array() {
        this(8);
    }

    public Array(int capacity) {
        data = (T[]) new Object[capacity];
        size = 0;
    }

    public final T set(int index, T entry) {
        if (size != 0) while (index < 0) index += size;
        ensureCapacity(index);
        T temp = data[index];
        data[index] = entry;
        if (temp == null) size++;
        return temp;
    }

    public int add(T entry) {
        ensureCapacity(size);
        data[size] = entry;
        size++;
        return size - 1;
    }

    public T get(int index) {
        if (index >= data.length) return null;
        if (data.length != 0) while (index < 0) index += data.length;
        return data[index];
    }

    public final void ensureCapacity(int minCapacity) {
        if (minCapacity < data.length) return;
        int newCap = (minCapacity * 3 / 2 + 1);
        T[] newData = (T[]) new Object[newCap];
        for (int i = 0; i < data.length; i++)
            newData[i] = data[i];
        data = newData;
    }

    public int size() {
        return size;
    }

    public final Iterator<T> filteredIterator(Filter<T> filter) {
        return new Iterator<T>() {
            private int index = 0;
            private T next;

            @Override
            public boolean hasNext() {
                while (index < data.length) {
                    if (data[index] != null
                            && filter.matches(data[index])) {
                        next = data[index];
                        index++;
                        return true;
                    }
                }
                return false;
            }

            @Override
            public T next() {
                return next;
            }
        };
    }
}
