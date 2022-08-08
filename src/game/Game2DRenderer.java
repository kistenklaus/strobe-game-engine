package game;

import org.lwjgl.opengl.GL11;
import org.lwjgl.opengl.GL13;
import org.lwjgl.opengl.GL20;
import org.lwjgl.opengl.GL30;

import engine.gfx.entity.Entity2D;
import engine.gfx.models.Model2D;
import engine.gfx.renderer.Renderer2D;
import engine.gfx.toolbox.Maths;

public class Game2DRenderer extends Renderer2D{
	private GameShader2D shader;
	public Game2DRenderer(GameShader2D shader) {
		super();
		this.shader = shader;
	}

	@Override
	protected void startShader() {
		this.shader.start();
	}

	
	@Override
	protected void processScene() {
		
	}

	@Override
	protected void bindModel(Model2D model) {
		GL30.glBindVertexArray(model.getModelVAO());
		GL20.glEnableVertexAttribArray(0);
		GL20.glEnableVertexAttribArray(1);
		GL13.glActiveTexture(GL13.GL_TEXTURE0);
		GL11.glBindTexture(GL11.GL_TEXTURE_2D, model.getTextureID());
	}

	@Override
	protected void prepareEntity(Entity2D entity) {
		this.shader.loadTransformationMatrix(Maths.create2DTransformationMatrix(entity.getPos(), entity.getRotation(), entity.getScale()));
	}
	@Override
	protected void unbindModel() {
		GL20.glDisableVertexAttribArray(0);
		GL20.glDisableVertexAttribArray(1);
		GL30.glBindVertexArray(0);
	}

	@Override
	protected void stopShader() {
		this.shader.stop();
	}

	@Override
	public void cleanUp() {
		this.cleanUp();
	}



}
