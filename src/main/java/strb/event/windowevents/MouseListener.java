package strb.event.windowevents;

import org.lwjgl.glfw.*;

public class MouseListener {
	
	private GLFWCursorPosCallbackI posCallback;
	private GLFWMouseButtonCallbackI buttonCallback;
	private GLFWScrollCallbackI scrollCallback;
	
	public MouseListener() {
		this.posCallback = new PosCallback();
		this.buttonCallback = new ButtonCallback();
		this.scrollCallback = new ScrollCallback();
	}
	
	public GLFWCursorPosCallbackI getPosCallback() {
		return posCallback;
	}
	public GLFWMouseButtonCallbackI getButtonCallback() {
		return buttonCallback;
	}
	public GLFWScrollCallbackI getScrollCallback() {
		return scrollCallback;
	}
	
	private class PosCallback implements GLFWCursorPosCallbackI{

		@Override
		public void invoke(long window, double xpos, double ypos) {
			
		}
		
	}
	private class ButtonCallback implements GLFWMouseButtonCallbackI{

		@Override
		public void invoke(long window, int button, int action, int mods) {
			
		}
		
	}
	private class ScrollCallback implements GLFWScrollCallbackI{

		@Override
		public void invoke(long window, double xoffset, double yoffset) {
			
		}
		
	}
	
}
