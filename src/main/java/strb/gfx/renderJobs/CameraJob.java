package strb.gfx.renderJobs;

import static org.lwjgl.opengl.GL11.GL_COLOR_BUFFER_BIT;
import static org.lwjgl.opengl.GL11.GL_NEAREST;
import static org.lwjgl.opengl.GL11.glClear;
import static org.lwjgl.opengl.GL30.GL_FRAMEBUFFER;
import static org.lwjgl.opengl.GL30.glBindFramebuffer;
import static org.lwjgl.opengl.GL45.glBlitNamedFramebuffer;
import strb.ecs.components.Camera;
import strb.gfx.UBO;

public class CameraJob extends RenderJob{
	
	private Camera camera;
	private UBO cameraUBO;
	
	public CameraJob(Camera cameraComponet) {
		this.camera = cameraComponet;
	}
	
	protected void perform() {
		
		synchronized (camera) {
			cameraUBO.uniformMat4Direct("proj", camera.getProjectionMatrix());
			cameraUBO.uniformMat4Direct("view", camera.getViewMatrix());
		}
		
		glBindFramebuffer(GL_FRAMEBUFFER, camera.getRenderFramebuffer().getID());
		
		glClear(GL_COLOR_BUFFER_BIT);
		
		for(RenderJob job : before) {
			job.execute();
		}
		render();
		for(RenderJob job : after) {
			job.execute();
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		
		glBlitNamedFramebuffer(camera.getRenderFramebuffer().getID(), 0, 0, 0, 800, 600, 0, 0, 800, 600, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	}
	
	public void setCameraUBO(UBO cameraUBO) {
		this.cameraUBO = cameraUBO;
	}
	
	@Override
	protected void render() {
		
	}
	
	
}
