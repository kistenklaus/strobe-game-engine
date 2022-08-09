package marshal;

import java.util.ArrayList;

class MarshalSignal<T>{
	
	private ArrayList<MarshalListener<T>> listeners = new ArrayList<>();
	
	void add(MarshalListener<T> listener) {
		listeners.add(listener);
	}
	boolean remove(MarshalListener<T> listener) {
		return listeners.remove(listener);
	}
	void dispatch(T obj) {
		for(MarshalListener<T> listener : listeners) {
			listener.envoke(obj);
		}
	}
}
