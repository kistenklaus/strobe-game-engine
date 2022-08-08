package engine;

import engine.gfx.Loader;

public abstract class Logic {
	protected final int WIDTH, HEIGHT;
	protected final String TITLE;
	protected Engine engine = null;
	public Logic(int WIDTH, int HEIGHT, String TITLE) {
		this.WIDTH = WIDTH;
		this.HEIGHT = HEIGHT;
		this.TITLE = TITLE;
	}
	public abstract void init(Loader loader);
	public abstract void repaint();
	public abstract void terminate();
	public int getWidth() {
		return this.WIDTH;
	}
	public int getHeight() {
		return this.HEIGHT;
	}
	public String getTitle() {
		return this.TITLE;
	}
	public void setEngine(Engine engine) {
		this.engine = engine;
	}
}
