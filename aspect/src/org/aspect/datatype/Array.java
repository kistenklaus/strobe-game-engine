package org.aspect.datatype;

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

    public T set(int index, T entry) {
        if(entry == null)throw new NullPointerException("Array does not accept null as an entry");
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

    public T get(int index){
        if(index>=data.length)throw new IndexOutOfBoundsException();
        if(data.length!=0)while(index<0)index+=data.length;
        return data[index];
    }

    public void ensureCapacity(int minCapacity) {
        if (minCapacity < data.length) return;
        int newCap = (minCapacity * 3 / 2 + 1);
        T[] newData = (T[]) new Object[newCap];
        for (int i = 0; i < newCap; i++)
            newData[i] = data[i];
        data = newData;
    }
}
