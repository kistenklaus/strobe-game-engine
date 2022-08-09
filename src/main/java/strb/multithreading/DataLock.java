package strb.multithreading;

import strb.event.Listener;
import strb.event.Signal;

public class DataLock<T> extends Lock{
	
	private T data;
	
	private Signal<T> unlockSignal = new Signal<>();
	
	public DataLock() {
		super(false);
	}
	
	public void unlock(T data) {
		this.data = data;
		unlockSignal.dispatch(data);
		super.unlock();
	}
	
	public synchronized void onUnlockData(Listener<T> listener) {
		unlockSignal.add(listener);
		if(open)unlockSignal.dispatch(data);
	}
	
	public Signal<T> getUnlockSignal() {
		return unlockSignal;
	}
	
	public T getData() {
		return data;
	}
	
	public void sync(DataLock<T> lock) {
		this.open = lock.open;
		lock.lockObject = this;
	}
	
}
