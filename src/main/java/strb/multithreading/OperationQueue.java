package strb.multithreading;

import java.util.ArrayList;
import java.util.Iterator;
import strb.gfx.renderJobs.RenderJob;

public class OperationQueue<T> implements Iterable<T>{

	public ArrayList<Operation<T>> operations = new ArrayList<>();
	
	public void push(T target, int priority) {
		for(int i=0;i<operations.size();i++) {
			if(operations.get(i).priority<=priority) {
				operations.add(i, new Operation<T>(target, priority));
				return;
			}
		}
		operations.add(new Operation<T>(target, priority));
	}
	
	public boolean isEmpty() {
		return operations.size() == 0;
	}
	
	private static class Operation<E> {
		private E target;
		private int priority = 0;
		private Operation(E target, int priority) {
			this.target = target;
			this.priority = priority;
		}
	}

	@Override
	public Iterator<T> iterator() {
		return new Iterator<T>() {
			private int i = 0;
			@Override
			public boolean hasNext() {
				return i<operations.size();
			}

			@Override
			public T next() {
				return operations.get(i++).target;
			}
		};
	}
	
	public int size() {
		return operations.size();
	}

	public void merge(OperationQueue<T> pending) {
		for(int i=0;i<pending.size();i++) {
			push(pending.operations.get(i).target, pending.operations.get(i).priority);
		}
		
	}

	public void clear() {
		operations.clear();
	}

	/**
	 * In theory here O(logn) would be possible when using the priority with a binary Search
	 * @param renderJob
	 */
	public synchronized void remove(RenderJob renderJob) {
		for(int i=0; i<operations.size();i++) {
			if(operations.get(i).target.equals(renderJob)) {
				operations.remove(i);
				return;
			}
		}
	}
}
