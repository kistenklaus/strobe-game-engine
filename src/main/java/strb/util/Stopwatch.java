package strb.util;


public class Stopwatch {
	
	private long start;
	
	public void start() {
		start = System.nanoTime();
	}
	
	public float end() {
		return (System.nanoTime()-start)*1e-9f;
	}
	
}
