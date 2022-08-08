package engine;

import org.lwjgl.opengl.GL;
import org.lwjgl.opengl.GL11;

import engine.gfx.Loader;
import engine.gfx.renderer.MasterRenderer;
import engine.gfx.renderer.Renderer;
import engine.input.Input;
import engine.input.InputManager;
import engine.io.Window;

public class Engine {
	private Window window;
	private MasterRenderer renderer;
	private Loader loader;
	private InputManager inputManager;
	private Input input;
	private Logic logic;
	
	public Engine(Logic logic) {
		this.logic = logic;
		this.logic.setEngine(this);
		this.window = new Window(logic.getWidth(), logic.getHeight(), logic.getTitle(), 60);
		GL.createCapabilities();	
		//Everthing OpenGL related needs to be behind GL.createCapabilities() and the Window needs to be created befor calling this
		this.renderer = new MasterRenderer();
		this.loader = new Loader();
		this.inputManager = new InputManager();
		this.window.setKeyCallback(inputManager.getKeyCallback());
		this.window.setMouseButtonCallback(inputManager.getMouseButtonCallback());
		this.window.setMousePositionCallback(inputManager.getMousePosCallback());
		this.window.setScrollCallback(inputManager.getMouseScrollCallback());
		
		this.input = new Input() {@Override public void update() {}};
		
		logic.init(this.loader);
	}
	
	public void start() {
		while(!this.window.shouldClose()) {
			if(this.window.windowShouldRepaint()) {
				this.window.pollEvents();
				input.update();
				this.renderer.processScenes();
				logic.repaint();
				this.renderer.render();
				this.window.swapBuffers();
			}
		}
		logic.terminate();
		loader.cleanUp();
		this.window.terminate();
	}
	public Window getWindow() {
		return this.window;
	}
	public void setInput(Input input) {
		input.setInputManager(this.inputManager);
		this.input = input;
	}
	public InputManager getInputManager() {
		return this.inputManager;
	}
	public void addRenderer(Renderer renderer) {
		this.renderer.addRenderer(renderer);
	}
}
