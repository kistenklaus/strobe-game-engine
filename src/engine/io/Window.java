package engine.io;
import static org.lwjgl.glfw.GLFW.*;

import org.lwjgl.glfw.GLFWCursorPosCallbackI;
import org.lwjgl.glfw.GLFWKeyCallbackI;
import org.lwjgl.glfw.GLFWMouseButtonCallbackI;
import org.lwjgl.glfw.GLFWScrollCallbackI;

public class Window {
	private long window;
	private long last;
	private double lastRepaint;
	private final double INV_FPS;
	public Window(int width, int height, String title, int FPS) {
		this.window = glfwCreateWindow(width, height, title, 0 ,0 );
		if(this.window == 0) {System.err.println("Error: Failed to create a Window");System.exit(-1);}
		glfwMakeContextCurrent(this.window);
		glfwShowWindow(this.window);
		last = System.nanoTime();
		INV_FPS = 1d/FPS;
	}
	public boolean windowShouldRepaint() {
		long curr = System.nanoTime();
		lastRepaint+=(curr - last)/ 1000000000d;
		last = curr;
		if(lastRepaint>INV_FPS) {
			lastRepaint -= INV_FPS;
			return true;
		}
		return false;
		
	}
	public boolean shouldClose() {
		return glfwWindowShouldClose(this.window);
	}
	public void pollEvents() {
		glfwPollEvents();
	}
	public void swapBuffers() {
		glfwSwapBuffers(this.window);
	}
	public void close() {
		glfwSetWindowShouldClose(this.window, true);
	}
	public void terminate() {
		glfwTerminate();
	}
	public void setKeyCallback(GLFWKeyCallbackI keyCallback) {
		glfwSetKeyCallback(this.window, keyCallback);
	}
	public void setMouseButtonCallback(GLFWMouseButtonCallbackI mouseCallback) {
		 glfwSetMouseButtonCallback(this.window, mouseCallback);
	}
	public void setMousePositionCallback(GLFWCursorPosCallbackI callback) {
		glfwSetCursorPosCallback(this.window, callback);
	}
	public void setScrollCallback(GLFWScrollCallbackI callback) {
		glfwSetScrollCallback(this.window, callback);
	}
	
}
