package engine.gfx.renderer;

public abstract class Renderer {
	public abstract void enableRenderSetting();
	public abstract void render();
	public abstract void disableRenderSetting();
	public abstract void cleanUp();
}
