package strb.ecs.components;

import org.joml.*;
import strb.ecs.Component;
import strb.gfx.Framebuffer;

public class Camera implements Component{
	
	private Mode mode;
	
	private Framebuffer renderFramebuffer = null;
	
	private int multisamples = 1;
	
	private Vector2f nearFarPlane = new Vector2f(0.0f, 100.0f);
	private Vector2f size;
	private Vector2i resolution;
	
	private Matrix4f projection;
	private Matrix4f view = new Matrix4f().identity();
	
	public Camera(float width, float height, int horizontalResolution, int verticalResolution) {
		size = new Vector2f(width, height);
		resolution = new Vector2i(horizontalResolution, verticalResolution);
		mode = Mode.ORTHO;
		projection = new Matrix4f().identity();
	}
	
	public enum Mode{
		ORTHO;
	}
	
	/*
	 * GETTERS AND SETTERS
	 */
	
	public Vector2i getResolution() {
		return resolution;
	}

	public Vector2f getSize() {
		return size;
	}

	public Mode getMode() {
		return mode;
	}
	
	public Matrix4f getProjectionMatrix() {
		return projection;
	}
	
	public Matrix4f getViewMatrix() {
		return view;
	}
	
	public Vector2f getNearFarPlane() {
		return nearFarPlane;
	}
	
	public Framebuffer getRenderFramebuffer() {
		return renderFramebuffer;
	}
	public boolean isCompleted() {
		return renderFramebuffer != null;
	}

	public int getMultisamples() {
		return multisamples;
	}

	public void setRenderFramebuffer(Framebuffer renderFbo) {
		renderFramebuffer = renderFbo;
	}


}
