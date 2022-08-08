package game.terrrain;

import java.util.ArrayList;
import java.util.List;

import org.joml.Vector3f;
import org.lwjgl.opengl.GL11;
import org.lwjgl.opengl.GL13;
import org.lwjgl.opengl.GL20;
import org.lwjgl.opengl.GL30;

import engine.gfx.entity.Light;
import engine.gfx.models.RawModel;
import engine.gfx.renderer.Renderer;
import engine.gfx.textures.ModelMaterial;
import engine.gfx.toolbox.Maths;
import game.Camera;

public class TerrainRenderer extends Renderer{
	
	private TerrainShader shader;
	private Light light;
	private Camera camera;
	private List<Terrain> terrains; 
	
	public TerrainRenderer(TerrainShader shader, Light light, Camera camera) {
		this.shader = shader;
		this.terrains = new ArrayList<>();
		this.light = light;
		this.camera = camera;
	}
	
	@Override
	public void processScene() {
		this.shader.start();
		this.shader.loadLight(light);
		this.shader.loadViewMatrix(camera.getPos(), camera.getPitch(), camera.getYaw(), camera.getRoll());
		this.shader.stop();
	}
	
	public void processTerrain(List<Terrain> terrains) {
		this.terrains = terrains;
	}

	@Override
	public void render() {
		this.shader.start();
		for (Terrain terrain : terrains) {
			prepareTerrain(terrain);
			loadModelMatrix(terrain);
			GL11.glDrawElements(GL11.GL_TRIANGLES, terrain.getModel().getVertexCount(), GL11.GL_UNSIGNED_INT, 0);
			unbindTexModel();
		}
		this.shader.stop();
	}
	
	private void prepareTerrain(Terrain terrain) {
		RawModel rawModel = terrain.getModel();
		GL30.glBindVertexArray(rawModel.getVaoID());
		GL20.glEnableVertexAttribArray(0);
		GL20.glEnableVertexAttribArray(1);
		GL20.glEnableVertexAttribArray(2);
		ModelMaterial material = terrain.getMaterial();
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
	
	private void loadModelMatrix(Terrain terrain) {
		this.shader.loadTransformationMatrix(Maths.createTransformationMatrix(
				new Vector3f(terrain.getX(), 0, terrain.getZ()), 0, 0, 0, 1));
		
	}

	@Override
	public void cleanUp() {
		
	}



}
