package engine.io;
import static org.lwjgl.glfw.GLFW.*;

import org.lwjgl.glfw.GLFW;
import org.lwjgl.glfw.GLFWCursorPosCallbackI;
import org.lwjgl.glfw.GLFWKeyCallbackI;
import org.lwjgl.glfw.GLFWMouseButtonCallbackI;
import org.lwjgl.glfw.GLFWScrollCallbackI;

public class Window {
	private long window;
	
	public static void initGLFW() {
		if(!GLFW.glfwInit()) {System.err.println("ERROR: GLFW Failed to Initalize");}
	}
	
	public Window(int width, int height, String title) {
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);
		this.window = glfwCreateWindow(width, height, title, 0 ,0 );
		if(this.window == 0) {System.err.println("Error: Failed to create a Window");System.exit(-1);}
		glfwMakeContextCurrent(this.window);
		glfwShowWindow(this.window);
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
