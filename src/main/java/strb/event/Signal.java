package strb.event;

import java.util.*;

public class Signal<T> {
	
	private ArrayList<Listener<T>> listeners;
	
	public Signal() {
		listeners = new ArrayList<>();
	}
	
	public Signal<T> add(Listener<T> listener) {
		listeners.add(0, listener);
		return this;
	}
	
	public void remove(Listener<T> listener) {
		listeners.remove(listener);
	}
	
	public void removeAllListeners() {
		listeners.clear();
	}
	
	public void dispatch(T source) {
		for(int i=listeners.size()-1; i >= 0; i--) {
			Listener<T> listener = listeners.get(i);
			listener.receive(this, source);
		}
	}
	
	public static Signal<Boolean> join(Signal<?>...signals){
		boolean[] envoked = new boolean[signals.length];
		HashMap<Signal<?>, Integer> signalIndex = new HashMap<>();
		Signal<Boolean> signalJoined = new Signal<Boolean>();
		
		for(int i=0;i<signals.length;i++) {
			signalIndex.put(signals[i], i);
			signals[i].add((signal, object) -> {
				envoked[signalIndex.get(signal)] = true;
				boolean joined = true;
				for(int j=0;j<envoked.length;j++) {
					if(!envoked[j]) {
						joined = false;
						break;
					}
				}
				if(joined) {
					signalJoined.dispatch(true);
					for(int j=0;j<envoked.length;j++) {
						envoked[j] = false;
					}
				}
			});
		}
		return signalJoined;
	}
	
	public static <T extends Object> Signal<Boolean> join(List<Signal<T>> signals){
		boolean[] envoked = new boolean[signals.size()];
		HashMap<Signal<?>, Integer> signalIndex = new HashMap<>();
		Signal<Boolean> signalJoined = new Signal<Boolean>();
		
		for(int i=0;i<signals.size();i++) {
			signalIndex.put(signals.get(i), i);
			signals.get(i).add((signal, object) -> {
				envoked[signalIndex.get(signal)] = true;
				boolean joined = true;
				for(int j=0;j<envoked.length;j++) {
					if(!envoked[j]) {
						joined = false;
						break;
					}
				}
				if(joined) {
					signalJoined.dispatch(true);
					for(int j=0;j<envoked.length;j++) {
						envoked[j] = false;
					}
				}
			});
		}
		return signalJoined;
	}
}
