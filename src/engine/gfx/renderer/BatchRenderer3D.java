package engine.gfx.renderer;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import org.lwjgl.opengl.GL11;

import engine.gfx.entity.Camera;
import engine.gfx.entity.Entity3D;
import engine.gfx.entity.Light3D;
import engine.gfx.models.Model3D;

public abstract class BatchRenderer3D extends Renderer3D{
	private HashMap<Model3D, List<Entity3D>> entities;
	public BatchRenderer3D(Camera camera, Light3D light) {
		super(camera, light);
		this.entities = new HashMap<>();
	}
	/**
	 * should just start the Shader
	 */
	protected abstract void startShader();
	/**
	 * should just stop the Shader
	 */
	protected abstract void stopShader();
	/**
	 * should upload all uniforms for the Scene like viewmatrix usw.
	 */
	protected abstract void processScene();
	/**
	 * Prepares a texturedModel to render all instances with it
	 * should just bind the Texture and activate all Attirbarrays, bind VAO, usw.
	 * @param model
	 */
	protected abstract void prepareTexModel(Model3D model);
	/**
	 * prepares a spec Entity3D for renderering
	 * @param entity
	 */
	protected abstract void prepareInstance(Entity3D entity);
	/**
	 * unbinds a TexturedModel
	 * should deactivate all Attribarrays unbind VAO usw.
	 */
	protected abstract void unbindTexturedModel();
	
	@Override
	public void render() {
		startShader();
		processScene();
		for (Model3D model : this.entities.keySet()) {
			prepareTexModel(model);
			List<Entity3D> batch = entities.get(model);
			for (Entity3D entity : batch) {
				prepareInstance(entity);
				GL11.glDrawElements(GL11.GL_TRIANGLES, model.getRawModel().getVertexCount(), GL11.GL_UNSIGNED_INT, 0);
			}
			unbindTexturedModel();
		}
		stopShader();
		this.entities.clear();
	}
	
	public void processEntity(Entity3D entity) {
		Model3D key = entity.getTex_model();
		List<Entity3D> batch = entities.get(key);
		if(batch != null) {
			batch.add(entity);
		}else {
			List<Entity3D> newBatch = new ArrayList<>();
			newBatch.add(entity);
			this.entities.put(key, newBatch);
		}
	}
	

}
