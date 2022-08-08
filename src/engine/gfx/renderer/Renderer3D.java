package engine.gfx.renderer;

import org.lwjgl.opengl.GL11;

import engine.gfx.entity.Camera;
import engine.gfx.entity.Light3D;

public abstract class Renderer3D extends Renderer{
	protected Camera camera;
	protected Light3D light;
	public Renderer3D(Camera camera, Light3D light) {
		this.camera = camera;
		this.light = light;
	}
	public Camera getCamera() {
		return camera;
	}
	public Light3D getLight() {
		return light;
	}
	public void enableRenderSetting() {
		GL11.glEnable(GL11.GL_DEPTH_TEST);
	}
	public void disableRenderSetting() {
		GL11.glDisable(GL11.GL_DEPTH_TEST);
	}
}
