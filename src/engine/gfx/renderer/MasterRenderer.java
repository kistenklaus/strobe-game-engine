package engine.gfx.renderer;

import java.util.ArrayList;
import java.util.List;

import org.joml.Vector3f;
import org.lwjgl.opengl.GL11;

public class MasterRenderer {
	public static Vector3f SkyColor = new Vector3f(0.5f, 0.5f, 0.5f);
	public static void setSkyColor(Vector3f skyColor) {
		SkyColor = skyColor;GL11.glClearColor(MasterRenderer.SkyColor.x, MasterRenderer.SkyColor.y, MasterRenderer.SkyColor.z, 1f);}
	
	private List<Renderer> renderers;
	public MasterRenderer() {
		this.renderers = new ArrayList<>();
		GL11.glClearColor(SkyColor.x, SkyColor.y, SkyColor.z, 1f);
		
	}
	
	public void render() {
		enableCulling();
		GL11.glClear(GL11.GL_COLOR_BUFFER_BIT | GL11.GL_DEPTH_BUFFER_BIT);
		for (Renderer renderer : renderers) {
			renderer.enableRenderSetting();
			renderer.render();
			renderer.disableRenderSetting();
		}
	}
	public static void enableCulling() {
		GL11.glEnable(GL11.GL_CULL_FACE);
		GL11.glCullFace(GL11.GL_BACK);
	}
	public static void disableCulling() {
		GL11.glDisable(GL11.GL_CULL_FACE);
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
