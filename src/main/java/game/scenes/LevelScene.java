package game.scenes;

import static org.lwjgl.glfw.GLFW.GLFW_KEY_SPACE;
import strb.ecs.Entity;
import strb.ecs.components.Camera;
import strb.ecs.components.Transform;
import strb.ecs.systems.CameraRenderer;
import strb.gfx.Batch;
import strb.gfx.VAO;
import strb.gfx.VAO.Data.Quad;
import strb.gfx.renderJobs.VaoRenderJob;
import strb.logic.Scene;

public class LevelScene extends Scene{
	
	
	@Override
	public void preload() throws InterruptedException {
		//Setup loading screen:
		engine.getVAOBuilder().createVAO("FSvao", new VAO.Data.Quad(1,1));
		engine.getShaderBuilder().compileShader("assets/shaders/ls");
		engine.getTextureBuilder().loadTexture("assets/resources/loadingScreen.png", true);

		System.out.println("waiting for loading to complete");
		engine.waitForPendingJobs();
		System.out.println("loading complete");
		
		engine.addGfxJob(new VaoRenderJob(
				engine.getVAOBuilder().getVAO("FSvao"), 
				engine.getShaderBuilder().getShader("assets/shaders/ls"),
				engine.getTextureBuilder().getTexture("assets/resources/loadingScreen.png", true)), 0);
		
		engine.waitForPendingJobs();
		
		engine.getShaderBuilder().compileShader("assets/shaders/default");
		engine.getVAOBuilder().createVAO("quad", new VAO.Data.Quad(100,100));
		engine.getTextureBuilder().loadTexture("assets/resources/BallsOfSteel.png", true);
		engine.getVAOBuilder().createBatch("batch", 8, 1.5f, true, "vec3", "vec2");


		System.out.println("waiting for loading to complete");
		engine.waitForPendingJobs();
		System.out.println("loading complete");
	}

	private Quad quad;
	private Batch batch;
	
	@Override
	public void start() {
		
		Entity camera = new Entity();
		camera.add(new Camera(400,300,800,600));
		camera.add(new Transform());
		addEntity(camera);
		
		batch = engine.getVAOBuilder().getBatch("batch");
		quad = new Quad(100,100);
		
		batch.insertLater(quad);
		
		VaoRenderJob rJob = new VaoRenderJob(
				batch,
				engine.getShaderBuilder().getShader("assets/shaders/default"),
				engine.getTextureBuilder().getTexture("assets/resources/BallsOfSteel.png", true));
		
		CameraRenderer cameraRenderer = new CameraRenderer(engine);
		
		addSystem(cameraRenderer);
		
		cameraRenderer.addToCamera(camera, rJob, 0);
		
	}
	
	private float width = 100;
	private float height = 100;
	public void update(float dt) {
		if(engine.getKeyListener().isKeyDown(GLFW_KEY_SPACE)) {
			width+= 10000*dt*(Math.random()-0.5f);
			height+= 10000*dt*(Math.random()-0.5f);
			width = Math.max(Math.min(width, 200) , 0);
			height = Math.max(Math.min(height, 200) , 0);
			quad.updateSize(width, height);
			batch.updateLater(quad);
		}
	}
}
