package engine.input;

import org.lwjgl.glfw.GLFW;
import org.lwjgl.glfw.GLFWCursorPosCallbackI;
import org.lwjgl.glfw.GLFWKeyCallbackI;
import org.lwjgl.glfw.GLFWMouseButtonCallbackI;
import org.lwjgl.glfw.GLFWScrollCallbackI;

public class InputManager {
	private double mouseX, mouseY, scrollX, scrollY;
	private KeyCallback keyCallback;
	private MousePosCallback mousePosCallback;
	private MouseButtonCallback mouseButtonCallback;
	private MouseScrollCallback mouseScrollCallback;
	private boolean[] keys;
	private boolean[] mouseButtons;
	public InputManager() {
		this.keyCallback = new KeyCallback();
		this.mousePosCallback = new MousePosCallback();
		this.mouseButtonCallback = new MouseButtonCallback();
		this.mouseScrollCallback = new MouseScrollCallback();
		this.keys = new boolean[GLFW.GLFW_KEY_LAST];
		this.mouseButtons = new boolean[GLFW.GLFW_MOUSE_BUTTON_LAST];
	}
	public class KeyCallback implements GLFWKeyCallbackI{
		@Override
		public void invoke(long window, int key, int scancode, int action, int mods) {
			if(action == 1)keys[key] = true;
			else if(action == 0)keys[key] = false;
		}
	}
	public class MousePosCallback implements GLFWCursorPosCallbackI{
		@Override
		public void invoke(long window, double x, double y) {
			mouseX = x;
			mouseY = y;
		}
	}
	public class MouseButtonCallback implements GLFWMouseButtonCallbackI{
		@Override
		public void invoke(long window, int button, int action, int mods) {
			if(action == 1)mouseButtons[button] = true;
			else if(action == 0)mouseButtons[button] = false;
		}
	}
	public class MouseScrollCallback implements GLFWScrollCallbackI{
		@Override
		public void invoke(long window, double dx, double dy) {
			scrollX+=dx;
			scrollY+=dy;
		}
	}
	public boolean isKeyDown(int keyCode) {
		return keys[keyCode];
	}
	public boolean isMouseButtonDown(int button) {
		return mouseButtons[button];
	}
	public KeyCallback getKeyCallback() {
		return this.keyCallback;
	}
	public MousePosCallback getMousePosCallback() {
		return this.mousePosCallback;
	}
	public MouseButtonCallback getMouseButtonCallback() {
		return this.mouseButtonCallback;
	}
	public MouseScrollCallback getMouseScrollCallback() {
		return mouseScrollCallback;
	}
	public double getMouseX() {
		return this.mouseX;
	}
	public double getMouseY() {
		return this.mouseY;
	}
	public double getScrollX() {
		return this.scrollX;
	}
	public double getScrollY() {
		return this.scrollY;
	}
	public void updateScroll() {
		this.scrollX = 0;
		this.scrollY = 0;
	}
}
