package strb.util;

import strb.event.Listener;
import strb.event.Signal;

public class IntervallTimer {
	
	private float intervall;
	private Signal<Float> intervallSignal;
	private long start, last;
	
	public IntervallTimer(float intervall, Listener<Float> onIntervall) {
		this.intervall = intervall;
		intervallSignal = new Signal<>();
		intervallSignal.add(onIntervall);
	}
	
	public void reset() {
		start = System.nanoTime();
		last = System.nanoTime();
	}
	
	public void update() {
		start = System.nanoTime();
		float dt = (start-last)*1e-9f;
		update(dt);
		last = start;
	}
	
	public void update(float dt) {
		if(dt >= intervall) {
			intervallSignal.dispatch(dt);
			intervall -= dt;
		}
	}
	
	
	
}
