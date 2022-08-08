package engine.gfx.renderer;

import engine.gfx.entity.Entity;

public abstract class Renderer {
	public abstract void renderScene();
	public abstract void processEntity(Entity entity);
	public abstract void render();
	public abstract void cleanUp();
}
