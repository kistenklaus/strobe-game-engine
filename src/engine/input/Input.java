package engine.input;

public abstract class Input {
	private InputManager inputHandler;
	public abstract void update();
	public boolean isKeyDown(int keyCode) {
		return inputHandler.isKeyDown(keyCode);
	}
	public boolean isMouseButtonDown(int button) {
		return inputHandler.isMouseButtonDown(button);
	}
	public double getMouseX() {
		return inputHandler.getMouseX();
	}
	public double getMouseY() {
		return inputHandler.getMouseY();
	}
	public double getScrollX() {
		return inputHandler.getScrollX();
	}
	public double getScrollY() {
		return inputHandler.getScrollY();
	}
	/**
	 * should be called after reading the scroll deltas
	 */
	public void updateScroll() {
		inputHandler.updateScroll();
	}
	public void setInputManager(InputManager inputManager) {
		this.inputHandler = inputManager;
	}
}
