package engine;

import org.lwjgl.glfw.GLFW;

import rubiksCube.RubiksLogic;

public class Boot {
	
	public static void main(String[] args) {
		if(!GLFW.glfwInit()) {
			System.err.println("ERROR: GLFW Failed to Initalize");
		}
		Engine engine = new Engine(new RubiksLogic());
		engine.start();
	}
	
}
