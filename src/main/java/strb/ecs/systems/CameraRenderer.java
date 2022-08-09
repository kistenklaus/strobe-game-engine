package strb.ecs.systems;

import java.util.HashMap;
import java.util.List;
import org.joml.Matrix4f;
import org.joml.Vector3f;
import strb.core.Console;
import strb.core.Engine;
import strb.ecs.*;
import strb.ecs.EntityComponentSystem.Event;
import strb.ecs.components.Camera;
import strb.ecs.components.Transform;
import strb.gfx.Framebuffer;
import strb.gfx.UBO;
import strb.gfx.renderJobs.CameraJob;
import strb.gfx.renderJobs.RenderJob;
import strb.gfx.renderJobs.RenderJob.Type;
import strb.multithreading.DataLock;
import strb.multithreading.Lock;

public class CameraRenderer extends EntitySystem{
	
	private List<Entity> cameraEntities;
	
	private HashMap<Entity, RenderJob> cameraJobs;
	
	private Engine engine;
	private ComponentMapper<Camera> cameraMapper = ComponentMapper.getFor(Camera.class);
	private ComponentMapper<Transform> transformMapper = ComponentMapper.getFor(Transform.class);
	private UBO cameraUBO;
	
	private DataLock<UBO> cameraUBOLock;
	
	public CameraRenderer(Engine engine) {
		this.engine = engine;
		cameraUBOLock = engine.getUboBuilder().create(toString(), "mat4 view", "mat4 proj");
		//Console.log("initUbobefore_"+cameraUBOLock.isLocked());
		cameraUBOLock.onUnlockData((s, camUbo)->{
			this.cameraUBO = camUbo;
			Console.log("ubo created:   " + camUbo);
		});
		Console.log("initUbo_"+cameraUBOLock.isLocked());
	}
	
	public void onAdd(EntityComponentSystem ecs) {
		cameraJobs = new HashMap<>();
		cameraEntities = ecs.getEntitiesFor(Family.all(Camera.class));
		ecs.on(Event.ENTITYADD, (s, entity)-> {
			addCamera(entity);
		});
		for(Entity entity : cameraEntities) {
			addCamera(entity);
		}
	}
	
	private void addCamera(Entity cameraEntity) {
		CameraJob cameraJob = new CameraJob(cameraMapper.get(cameraEntity));
		cameraJobs.put(cameraEntity, cameraJob);
		
		
		Camera camera = cameraMapper.get(cameraEntity);
		//Creates a Framebuffer for the Camera
		DataLock<Framebuffer> renderFboLock = engine.getFboAdapter().createFramebuffer(camera.toString(), 
				camera.getResolution().x, camera.getResolution().y,
				camera.getMultisamples(), false, false);
		renderFboLock.onUnlockData((s, renderFbo)->{
			camera.setRenderFramebuffer(renderFbo);
		});
		//when everyhing is loaded add the cameraJob
		Console.log("beforeUbo_" + cameraUBOLock.isLocked());
		Lock unlocked = Lock.join(renderFboLock, cameraUBOLock);
		unlocked.onUnlock((s,b)->{
			cameraJob.setCameraUBO(cameraUBO);
			engine.addRenderJob(cameraJob, 0);
			Console.log("joinUbo_"+cameraUBOLock.isLocked());
		});
		
	}
	
	public void onRemove(EntityComponentSystem ecs) {
		cameraJobs.forEach((entity, job)->{
			engine.removeRenderJob(job);
		});
		cameraJobs = null;
	}
	
	public void addToCamera(Entity cameraEntity, RenderJob renderJob, int priority) {
		cameraJobs.get(cameraEntity).sub(Type.BEFORE, renderJob, priority);
	}
	
	private void updateProjection(Camera camera) {
		
		float width = camera.getSize().x;
		float height = camera.getSize().y;
		float near = camera.getNearFarPlane().x;
		float far = camera.getNearFarPlane().y;
		
		Matrix4f projection = camera.getProjectionMatrix();
		synchronized (camera) {
			switch(camera.getMode()) {
				case ORTHO:
					projection.identity();
					projection.ortho(-width/2.0f, width/2.0f, -height/2.0f, height/2.0f, near, far);
					break;
				default:
					Console.error("undefined camera mode!!!");
			}
			
		}
	}
	
	private void updateViewMatrix(Camera camera, Transform transform) {
		synchronized (camera) {
			Matrix4f view = camera.getViewMatrix();
			view.identity();
			view.translate(transform.getPosition());
			view.translate(transform.getRotationCenter());
			view.rotate((float)Math.toRadians(transform.getRotation().z), new Vector3f(0,0,-1));
			view.translate(-transform.getRotationCenter().x, -transform.getRotationCenter().y, -transform.getRotationCenter().z);
			view.scale(transform.getScale());
			view.invert();
		}
	}
	
	public void update(float dt) {
		for(Entity cameraEntity : cameraEntities) {
			Camera camera = cameraMapper.get(cameraEntity);
			Transform transform = transformMapper.get(cameraEntity);
			updateProjection(camera);
			updateViewMatrix(camera, transform);
			
		}
	}
	
}
