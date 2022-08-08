package game;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import org.lwjgl.opengl.GL11;
import org.lwjgl.opengl.GL13;
import org.lwjgl.opengl.GL20;
import org.lwjgl.opengl.GL30;

import engine.gfx.entity.Entity;
import engine.gfx.entity.Light;
import engine.gfx.models.RawModel;
import engine.gfx.models.TexturedModel;
import engine.gfx.renderer.Renderer;
import engine.gfx.shaders.Shader3D;
import engine.gfx.textures.ModelMaterial;
import engine.gfx.toolbox.Maths;

public class Renderer3D extends Renderer{
	private Shader3D shader;
	private Camera camera;
	private Light light;
	private HashMap<TexturedModel, List<Entity>> entities;
	public Renderer3D(Shader3D shader, Light light, Camera camera) {
		this.shader = shader;
		this.light = light;
		this.camera = camera;
		this.entities = new HashMap<>();
	}
	
	@Override
	public void processScene() {
		this.shader.start();
		this.shader.loadLight(this.light);
		this.shader.loadViewMatrix(this.camera.getPos(), this.camera.getPitch(), this.camera.getYaw(), this.camera.getRoll());
		this.shader.stop();
		this.entities.clear();
	}
	
	public void processEntity(Entity entity) {
		TexturedModel key = entity.getTex_model();
		List<Entity> batch = entities.get(key);
		if(batch != null) {
			batch.add(entity);
		}else {
			List<Entity> newBatch = new ArrayList<>();
			newBatch.add(entity);
			this.entities.put(key, newBatch);
		}
	}
	
	@Override
	public void render() {
		shader.start();
		for (TexturedModel model : this.entities.keySet()) {
			prepareTexModel(model);
			List<Entity> batch = entities.get(model);
			for (Entity entity : batch) {
				prepareInstance(entity);
				GL11.glDrawElements(GL11.GL_TRIANGLES, model.getRawModel().getVertexCount(), GL11.GL_UNSIGNED_INT, 0);
			}
			unbindTexModel();
		}
		shader.stop();
	}
	
	private void prepareTexModel(TexturedModel texModel) {
		RawModel rawModel = texModel.getRawModel();
		GL30.glBindVertexArray(rawModel.getVaoID());
		GL20.glEnableVertexAttribArray(0);
		GL20.glEnableVertexAttribArray(1);
		GL20.glEnableVertexAttribArray(2);
		ModelMaterial material = texModel.getMaterial();
		this.shader.loadShineVariables(material.getShineDamper(), material.getReflectivity());
		GL13.glActiveTexture(GL13.GL_TEXTURE0);
		GL11.glBindTexture(GL11.GL_TEXTURE_2D, material.getID());
	}
	
	private void unbindTexModel() {
		GL20.glDisableVertexAttribArray(0);
		GL20.glDisableVertexAttribArray(1);
		GL20.glDisableVertexAttribArray(2);
		GL30.glBindVertexArray(0);
	}
	
	private void prepareInstance(Entity entity) {
		this.shader.loadTransformationMatrix(Maths.createTransformationMatrix(
				entity.getPos(), entity.getRotX(), entity.getRotY(), entity.getRotZ(), entity.getScale()));
		
	}
	
	@Override
	public void cleanUp() {
		shader.cleanUp();
	}
}
