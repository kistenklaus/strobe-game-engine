package strb.gfx;

import static org.lwjgl.glfw.GLFW.*;
import static org.lwjgl.system.MemoryUtil.NULL;
import org.lwjgl.glfw.*;
import org.lwjgl.opengl.GL;
import strb.core.Console;
import strb.event.windowevents.KeyListener;
import strb.event.windowevents.MouseListener;

public class Window {
	
	private long window;
	private boolean FULLSCREEN = false;
	private final int window_width, window_height;
	
	public Window(String title, int window_width, int window_height, boolean fullscreen) {
		
		glfwDefaultWindowHints();
		glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		this.FULLSCREEN = fullscreen;
		
		this.window_width = window_width;
		this.window_height = window_height;
		
		long monitor = NULL;
		if(FULLSCREEN) {
			monitor = glfwGetPrimaryMonitor();
			GLFWVidMode mode = glfwGetVideoMode(monitor);
			window_width = mode.width();
			window_height = mode.height();
		}
		window = glfwCreateWindow(window_width, window_height, title, monitor, NULL);
		if(window == NULL) {
			Console.error("Failed to create GLFW-Window");
		}
		
		glfwMakeContextCurrent(window);
		glfwSwapInterval(0);
		glfwShowWindow(window);
		GL.createCapabilities();
	}
	
	public boolean shouldClose() {
		return glfwWindowShouldClose(window);
	}
	public void close() {
		glfwSetWindowShouldClose(window, true);
	}
	
	public void addMouseListener(MouseListener mouseListener) {
		glfwSetCursorPosCallback(window, mouseListener.getPosCallback());
		glfwSetMouseButtonCallback(window, mouseListener.getButtonCallback());
		glfwSetScrollCallback(window, mouseListener.getScrollCallback());
	} 
	public void addKeyListener(KeyListener keyListener) {
		glfwSetKeyCallback(window, keyListener.getKeyCallback());
	}
	
	public void swapBuffers() {
		glfwSwapBuffers(window);
	}
	
	public int getWidth() {
		return window_width;
	}
	public int getHeight() {
		return window_height;
	}
	
}
