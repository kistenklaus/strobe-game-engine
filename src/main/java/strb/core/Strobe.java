package strb.core;

import strb.logic.Scene;

public class Strobe {
	
	public static void start(Scene scene){
		Engine engine = new Engine(scene);
		engine.start();
	}
}
