package engine;

public class Clock {
	private long last;
	private double lastRepaint;
	private final double INV_FPS;
	private double dT;
	
	public Clock(int FPS) {
		last = System.nanoTime();
		INV_FPS = 1d/FPS;
	}
	
	public boolean shouldRepaint() {
		long curr = System.nanoTime();
		lastRepaint+=(curr - last)/ 1000000000d;
		last = curr;
		if(lastRepaint>INV_FPS) {
			this.dT = lastRepaint;
			lastRepaint = 0;
			return true;
		}
		
		return false;
		
	}
	public double getDeltaTime() {
		return dT;
	}
}
