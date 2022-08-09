package strobe.datatypes;

import java.util.Arrays;
import java.util.Iterator;

public class Array<T> implements Iterable<T>{

    private T[] data;
    private int entries = 0;

    public Array(){
        this(8);
    }
    public Array(int capacity){
        if(capacity<4)capacity = 4;
        data = (T[]) new Object[capacity];
    }

    public int add(T t){
        entries++;
        for(int i=0;i<data.length;i++){
            if(data[i] == null) data[i] = t;
            return i;
        }
        int add_index = data.length;
        grow(data.length);
        data[add_index] = t;
        return add_index;
    }
    public T get(int index){
        if(index>=data.length)return null;
        while(index<0)index+=data.length;
        return data[index];
    }
    public void set(int index, T t){
        while(index<0)index+=data.length;
        if(index>=data.length)grow(index);
        if(data[index] == null)entries++;
        data[index] = t;
    }

    public T remove(int index){
        while(index<0)index+=data.length;
        if(index>=data.length)return null;
        T temp = data[index];
        data[index] = null;
        if(temp != null)entries--;
        return temp;
    }

    public int size(){
        return entries;
    }

    public void grow(int min_length){
        int new_length = (min_length * 3) / 2;
        T[] new_data = (T[]) new Object[new_length];
        for(int i=0;i<data.length;i++)new_data[i] = data[i];
        data = new_data;
    }

    @Override
    public Iterator<T> iterator() {
        return new Iterator<T>() {
            private int it_index = 0;
            private int it_entries_index = 0;
            @Override
            public boolean hasNext() {
                return it_entries_index<entries;
            }

            @Override
            public T next() {
                while(data[it_index] == null)it_index++;
                it_entries_index++;
                return data[it_index++];
            }
        };
    }
    public String toString(){
        return Arrays.toString(data);
    }
}
