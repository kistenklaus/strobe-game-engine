package game.terrrain;

import java.util.ArrayList;
import java.util.List;

import org.joml.Vector3f;
import org.lwjgl.opengl.GL11;
import org.lwjgl.opengl.GL13;
import org.lwjgl.opengl.GL20;
import org.lwjgl.opengl.GL30;

import engine.gfx.entity.Camera;
import engine.gfx.entity.Light3D;
import engine.gfx.models.RawModel3D;
import engine.gfx.renderer.MasterRenderer;
import engine.gfx.renderer.Renderer3D;
import engine.gfx.toolbox.Maths;

public class TerrainRenderer extends Renderer3D{
	
	private TerrainShader shader;
	private List<Terrain> terrains; 
	
	public TerrainRenderer(TerrainShader shader, Light3D light, Camera camera) {
		super(camera, light);
		this.shader = shader;
		this.terrains = new ArrayList<>();
	}
	
	@Override
	public void render() {
		this.shader.start();
		renderScene();
		for (Terrain terrain : terrains) {
			prepareTerrain(terrain);
			loadModelMatrix(terrain);
			GL11.glDrawElements(GL11.GL_TRIANGLES, terrain.getModel().getVertexCount(), GL11.GL_UNSIGNED_INT, 0);
			unbindTexModel();
		}
		this.shader.connectTexUnits();	
		this.shader.stop();
		
	}
	
	private void renderScene() {
		this.shader.loadLight(light);
		this.shader.loadViewMatrix(camera.getPos(), camera.getPitch(), camera.getYaw(), camera.getRoll());
		this.shader.loadSkyColor(MasterRenderer.SkyColor);
	}
	
	public void processTerrain(List<Terrain> terrains) {
		this.terrains = terrains;
	}
	
	private void prepareTerrain(Terrain terrain) {
		RawModel3D rawModel = terrain.getModel();
		GL30.glBindVertexArray(rawModel.getVaoID());
		GL20.glEnableVertexAttribArray(0);
		GL20.glEnableVertexAttribArray(1);
		GL20.glEnableVertexAttribArray(2);
		bindTextures(terrain);
		this.shader.loadShineVariables(1, 0);
	}
	
	private void bindTextures(Terrain terrain) {
		TerrainTexurePack texturepack = terrain.getTexturepack();
		GL13.glActiveTexture(GL13.GL_TEXTURE0);
		GL11.glBindTexture(GL11.GL_TEXTURE_2D, texturepack.getBgTex().getTexID());
		GL13.glActiveTexture(GL13.GL_TEXTURE1);
		GL11.glBindTexture(GL11.GL_TEXTURE_2D, texturepack.getrTex().getTexID());
		GL13.glActiveTexture(GL13.GL_TEXTURE2);
		GL11.glBindTexture(GL11.GL_TEXTURE_2D, texturepack.getbTex().getTexID());
		GL13.glActiveTexture(GL13.GL_TEXTURE3);
		GL11.glBindTexture(GL11.GL_TEXTURE_2D, texturepack.getgTex().getTexID());
		GL13.glActiveTexture(GL13.GL_TEXTURE4);
		GL11.glBindTexture(GL11.GL_TEXTURE_2D, terrain.getBlendMap().getTexID());
	}
	
	private void unbindTexModel() {
		GL20.glDisableVertexAttribArray(0);
		GL20.glDisableVertexAttribArray(1);
		GL20.glDisableVertexAttribArray(2);
		GL30.glBindVertexArray(0);
	}
	
	private void loadModelMatrix(Terrain terrain) {
		this.shader.loadTransformationMatrix(Maths.create3DTransformationMatrix(
				new Vector3f(terrain.getX(), 0, terrain.getZ()), 0, 0, 0, 1));
		
	}

	@Override
	public void cleanUp() {
		this.shader.cleanUp();
	}
}
