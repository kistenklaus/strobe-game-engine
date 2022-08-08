package engine.gfx.renderer;

import java.util.ArrayList;
import java.util.List;

import org.lwjgl.opengl.GL11;

public class MasterRenderer {
	private List<Renderer> renderers;
	public MasterRenderer() {
		this.renderers = new ArrayList<>();
	}
	public void renderScenes() {
		for (Renderer renderer : renderers) {
			renderer.renderScene();
		}
		GL11.glClear(GL11.GL_COLOR_BUFFER_BIT | GL11.GL_DEPTH_BUFFER_BIT);
	}
	public void render() {
		for (Renderer renderer : renderers) {
			renderer.render();
		}
	}
	public void cleanUp() {
		for (Renderer renderer : renderers) {
			renderer.cleanUp();
		}
	}
	public void addRenderer(Renderer renderer) {
		this.renderers.add(renderer);
	}
}
