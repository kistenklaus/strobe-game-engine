package strb.gfx.renderJobs;

import static org.lwjgl.opengl.GL11.GL_TEXTURE_2D;
import static org.lwjgl.opengl.GL11.GL_TRIANGLES;
import static org.lwjgl.opengl.GL11.GL_UNSIGNED_INT;
import static org.lwjgl.opengl.GL11.glBindTexture;
import static org.lwjgl.opengl.GL11.glDrawElements;
import static org.lwjgl.opengl.GL13.GL_TEXTURE0;
import static org.lwjgl.opengl.GL13.glActiveTexture;
import static org.lwjgl.opengl.GL20.glUseProgram;
import static org.lwjgl.opengl.GL30.glBindVertexArray;
import strb.gfx.*;

public class VaoRenderJob extends RenderJob{
	
	protected VAO vao;
	protected Shader shader;
	private Texture texture;
	
	public VaoRenderJob(VAO vao, Shader shader, Texture texture) {
		this.vao = vao;
		this.shader = shader;
		this.texture = texture;
	}
	
	@Override
	protected void render() {
		shaderSetup();
		if(texture!=null)textureSetup();
		renderCall();
	}
	
	protected void shaderSetup() {
		glUseProgram(shader.getID());
	}
	
	protected void textureSetup() {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture.getID());
		shader.uniform1i("TEXTURE_SAPLER", 0);
	}
	
	protected void renderCall() {
		glBindVertexArray(vao.getID());
		glDrawElements(GL_TRIANGLES, vao.getDrawCount(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}


	
}
