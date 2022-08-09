package marshal;

import java.util.Arrays;
import java.util.Iterator;

@SuppressWarnings("unchecked")
class MarshalStaticArray<T> implements Iterable<T>{
	
	private int entries = 0;
	private T[] data;
	
	MarshalStaticArray() {
		this(16);
	}
	MarshalStaticArray(int capacity) {
		data = (T[]) new Object[capacity];
	}
	int add(T obj) {
		int index = 0;
		while(index < data.length && data[index] != null) {
			index++;
		}
		set(index, obj);
		return index;
	}
	T set(int index, T obj) {
		if(index>=data.length) grow(index);
		T r = data[index];
		data[index] = obj;
		if(r == null)entries++;
		return r;
	}
	boolean remove(int index) {
		if(index>=data.length)return false;
		while(index<0)index+=data.length;
		if(data[index] != null) {
			data[index] = null;
			entries--;
			return true;
		}else {
			return false;
		}
	}
	boolean remove(T obj) {
		for(int i=0;i<data.length;i++) {
			T t = data[i];
			if(t != null && t.equals(obj)) {
				return remove(i);
			}
		}
		return false;
	}
	T get(int index) {
		if(index>=data.length) {
			return null;
		}
		while(index<0)index+=data.length;
		return data[index];
	}
	
	T computeIfNull(int index, Lambda<T> lambda) {
		T t = get(index);
		if(t != null) return t;
		t = lambda.invoke();
		if(t == null)throw new IllegalStateException("computeIfNull does not compute a value != null");
		entries++;
		data[index] = t;
		return t;
	}
	private void grow(int minLength) {
		int newLength = minLength/3/2+1;
		T[] ndata = (T[]) new Object[newLength];
		for(int i=0;i<data.length;i++) {
			ndata[i] = data[i];
		}
		data = ndata;
	}
	int size() {
		return entries;
	}	
	interface Lambda<T>{
		T invoke();
	}
	@Override
	public Iterator<T> iterator() {
		return new Iterator<T>() {
			int index = 0;
			int count = 0;
			@Override
			public boolean hasNext() {
				return count<entries;
			}

			@Override
			public T next() {
				while(data[index] == null)index++;
				count++;
				return data[index];
			}
		};
	}
	public String toString() {
		return Arrays.toString(data);
	}
}
