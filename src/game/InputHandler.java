package game;

import engine.input.Input;

public class InputHandler extends Input{
	private Camera camera;
	public InputHandler(Camera camera) {
		this.camera = camera;
	}
	
	@Override
	public void update() {
		this.camera.inputHandling(this);
	}

}
