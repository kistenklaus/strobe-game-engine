package strb.multithreading;

import strb.event.Listener;
import strb.event.Signal;

public class Lock {
	
	protected Boolean open = false;
	protected Lock lockObject;
	
	private Signal<Boolean> unlockSignal = new Signal<>();
	
	public Lock() {
		this(false);
	}
	
	public Lock(boolean open) {
		this.open = open;
		lockObject = this;
	}

	public void lock() {
		open = false;
	}
	
	public void unlock() {
		open = true;
		synchronized (lockObject) {
			notify();
			unlockSignal.dispatch(open);
		}
	}
	
	public void onUnlock(Listener<Boolean> listener) { 
		unlockSignal.add(listener);
	}
	
	public void waitForUnlock() throws InterruptedException {
		while(!open) {
			synchronized (lockObject) {
				wait();
			}
		}
	}
	
	public boolean isLocked() {
		return !open;
	}
	
	public void sync(Lock lock) {
		this.open = lock.open;
		lock.lockObject = this;
	}

	public static Lock join(Lock...locks) {
		Lock joined = new Lock();
		boolean[] unlocked = new boolean[locks.length];
		for(int i=0;i<locks.length;i++) {
			final int index = i;
			Lock lock = locks[index];
			synchronized (lock) {
				 if(lock.isLocked()){
					 unlocked[index] = false;
					 lock.onUnlock((s,b)->{
						 unlocked[index] = true;
						 boolean join = true;
						 for(Boolean unlock : unlocked) {
							 if(!unlock) {
								 join = false;
								 break;
							 }
						 }
						 if(join) {
							 joined.unlock();
							 return;
						 }
					 });
				 }else {
					 unlocked[i] = true;
				 }
			}
		}
		boolean join = true;
		for(Boolean unlock : unlocked) {
			if(!unlock) {
				join = false;
				break;
			}
		}
		if(join) {
			joined.unlock();
		}
		return joined;
	}
	
}
