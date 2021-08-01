package org.strobe.commons.lang;

import java.util.AbstractList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Iterator;

public class UnboundedArray<T> extends AbstractList<T> {

    private T[] array;

    public UnboundedArray(int size) {
        array = (T[]) new Object[size];
    }

    public void resize(int size) {
        if (size == array.length) return;
        T[] newArray = (T[]) new Object[size];
        int lim = Math.min(newArray.length, array.length);
        for (int i = 0; i < lim; i++) {
            newArray[i] = array[i];
        }
        array = newArray;
    }

    public void trim() {
        for (int i = array.length - 1; i >= 0; i++) {
            if (array[i] != null) {
                resize(i);
                return;
            }
        }
    }

    @Override
    public int size() {
        return array.length;
    }

    @Override
    public boolean isEmpty() {
        for (int i = 0; i < array.length; i++) {
            if (array[i] != null) return false;
        }
        return true;
    }

    @Override
    public boolean contains(Object o) {
        for (int i = 0; i < array.length; i++) {
            if (array[i] == o) return true;
        }
        return false;
    }

    @Override
    public Iterator<T> iterator() {
        return new ArrayIterator(array, false);
    }

    public Iterator<T> nullIterator() {
        return new ArrayIterator(array, true);
    }

    @Override
    public String toString() {
        StringBuilder str = new StringBuilder();
        str.append("[");
        Iterator<T> iterator = iterator();
        while(iterator.hasNext()){
            T next = iterator.next();
            str.append(next).append(", ");
        }
        if(!isEmpty())str.delete(str.length()-", ".length(), str.length());
        str.append("]");
        return str.toString();
    }

    @Override
    public T[] toArray() {
        return Arrays.copyOf(array, array.length);
    }

    @Override
    public <T1> T1[] toArray(T1[] a) {
        return Arrays.copyOf(a, a.length);
    }

    @Override
    public boolean add(T value) {
        if(value == null)throw new IllegalArgumentException("UnboundedArray doesn't accept null");
        int insertIndex = getNextFreeIndex();
        set(insertIndex, value);
        return true;
    }

    public int getNextFreeIndex(){
        for(int i=0;i<array.length;i++){
            if(array[i] == null)return i;
        }
        return array.length;
    }

    public T set(int index, T value) {
        if(index<0)throw new IndexOutOfBoundsException("negative index");
        if(index>=array.length){
            if(value == null)return null;
            resize(index*3/2+1);
        }
        T temp = array[index];
        array[index] = value;
        return temp;
    }

    public T get(int index){
        if(index<0)throw new IndexOutOfBoundsException("negative index");
        if(index>=array.length)return null;
        return array[index];
    }

    public int getIndexOf(Object o){
        if(o == null)throw new IllegalArgumentException("UnboundedArray doesn't accept null");
        for(int i=0;i<array.length;i++){
            if(array[i] == o)return i;
        }
        return -1;
    }

    @Override
    public boolean remove(Object o) {
        if (o == null) return false;
        for (int i = 0; i < array.length; i++) {
            if (array[i] == o) {
                array[i] = null;
                return true;
            }
        }
        return false;
    }

    public T remove(int index){
        if(index < 0)throw new IndexOutOfBoundsException("negative index");
        if(index >= array.length) return null;
        T temp = array[index];
        array[index] = null;
        return temp;
    }

    @Override
    public boolean containsAll(Collection<?> c) {
        return false;
    }

    @Override
    public boolean addAll(Collection<? extends T> c) {
        throw new UnsupportedOperationException();
    }

    @Override
    public boolean removeAll(Collection<?> c) {
        boolean changed = false;
        for (Object o : c) {
            if (remove(o)) changed = true;
        }
        return changed;
    }

    @Override
    public boolean retainAll(Collection<?> c) {
        boolean changed = false;
        for (Object o : array) {
            if (!c.contains(o)) {
                if (remove(o)) changed = true;
            }
        }
        return changed;
    }

    @Override
    public void clear() {
        for (int i = 0; i < array.length; i++) {
            array[i] = null;
        }
    }
}
