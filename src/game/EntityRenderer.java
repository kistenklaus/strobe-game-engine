package game;

import org.lwjgl.opengl.GL11;
import org.lwjgl.opengl.GL13;
import org.lwjgl.opengl.GL20;
import org.lwjgl.opengl.GL30;

import engine.gfx.entity.Camera;
import engine.gfx.entity.Entity3D;
import engine.gfx.entity.Light3D;
import engine.gfx.models.Model3D;
import engine.gfx.renderer.BatchRenderer3D;
import engine.gfx.renderer.MasterRenderer;
import engine.gfx.textures.Material;
import engine.gfx.toolbox.Maths;

public class EntityRenderer extends BatchRenderer3D{
	private EntityShader shader;
	private Camera camera;
	private Light3D light;
	public EntityRenderer(EntityShader shader, Light3D light, Camera camera) {
		super(camera, light);
		this.shader = shader;
		this.light = light;
		this.camera = camera;
		
	}
	
	@Override
	protected void startShader() {
		this.shader.start();
	}

	@Override
	protected void stopShader() {
		this.shader.stop();
	}

	@Override
	protected void processScene() {
		this.shader.loadLight(this.light);
		this.shader.loadViewMatrix(this.camera.getPos(), this.camera.getPitch(), this.camera.getYaw(), this.camera.getRoll());
		this.shader.loadSkyColor(MasterRenderer.SkyColor);
	}
	
	@Override
	protected void prepareTexModel(Model3D texModel) {
		GL30.glBindVertexArray(texModel.getRawModel().getVaoID());
		GL20.glEnableVertexAttribArray(0);
		GL20.glEnableVertexAttribArray(1);
		GL20.glEnableVertexAttribArray(2);
		Material material = texModel.getMaterial();
		if(material.hasTransparency()) {
			MasterRenderer.disableCulling();
		}
		this.shader.loadUseFakeLightingBoolean(material.hasFakeLighing());
		this.shader.loadShineVariables(material.getShineDamper(), material.getReflectivity());
		GL13.glActiveTexture(GL13.GL_TEXTURE0);
		GL11.glBindTexture(GL11.GL_TEXTURE_2D, material.getID());
	}
	
	@Override
	protected void prepareInstance(Entity3D entity) {
		this.shader.loadTransformationMatrix(Maths.create3DTransformationMatrix(
				entity.getPos(), entity.getRotX(), entity.getRotY(), entity.getRotZ(), entity.getScale()));
		
	}
	
	@Override
	protected void unbindTexturedModel() {
		MasterRenderer.enableCulling();
		GL20.glDisableVertexAttribArray(0);
		GL20.glDisableVertexAttribArray(1);
		GL20.glDisableVertexAttribArray(2);
		GL30.glBindVertexArray(0);
	}
	

	
	@Override
	public void cleanUp() {
		shader.cleanUp();
	}
}
