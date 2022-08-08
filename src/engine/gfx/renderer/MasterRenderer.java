package engine.gfx.renderer;

import java.util.ArrayList;
import java.util.List;

import org.lwjgl.opengl.GL11;

public class MasterRenderer {
	private List<Renderer> renderers;
	public MasterRenderer() {
		this.renderers = new ArrayList<>();
		
	}
	public void processScenes() {
		for (Renderer renderer : renderers) {
			renderer.processScene();
		}
	}
	public void render() {
		GL11.glEnable(GL11.GL_DEPTH_TEST);
		GL11.glEnable(GL11.GL_CULL_FACE);
		GL11.glCullFace(GL11.GL_BACK);
		GL11.glClear(GL11.GL_COLOR_BUFFER_BIT | GL11.GL_DEPTH_BUFFER_BIT);
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
