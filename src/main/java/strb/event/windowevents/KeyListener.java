package strb.event.windowevents;

import static org.lwjgl.glfw.GLFW.GLFW_KEY_LAST;
import static org.lwjgl.glfw.GLFW.GLFW_PRESS;
import static org.lwjgl.glfw.GLFW.GLFW_RELEASE;
import org.lwjgl.glfw.GLFWKeyCallbackI;

public class KeyListener {
	
	private GLFWKeyCallbackI keyCallback;
	
	private boolean[] keyDown = new boolean[GLFW_KEY_LAST];
	
	public KeyListener() {
		keyCallback = new KeyCallback();
	}
	
	public GLFWKeyCallbackI getKeyCallback() {
		return keyCallback;
	}
	
	public boolean isKeyDown(int key) {
		return keyDown[key];
	}
	
	private class KeyCallback implements GLFWKeyCallbackI{
		
		@Override
		public void invoke(long window, int key, int scancode, int action, int mods) {
			if(action == GLFW_PRESS) {
				keyDown[key] = true;
			}else if(action == GLFW_RELEASE) {
				keyDown[key] = false;
			}
		}
		
	}
	
}
