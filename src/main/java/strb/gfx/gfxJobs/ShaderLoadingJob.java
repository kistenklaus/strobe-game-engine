package strb.gfx.gfxJobs;

import static org.lwjgl.opengl.GL11.GL_FALSE;
import static org.lwjgl.opengl.GL20.GL_COMPILE_STATUS;
import static org.lwjgl.opengl.GL20.GL_FRAGMENT_SHADER;
import static org.lwjgl.opengl.GL20.GL_INFO_LOG_LENGTH;
import static org.lwjgl.opengl.GL20.GL_LINK_STATUS;
import static org.lwjgl.opengl.GL20.GL_VERTEX_SHADER;
import static org.lwjgl.opengl.GL20.glAttachShader;
import static org.lwjgl.opengl.GL20.glCompileShader;
import static org.lwjgl.opengl.GL20.glCreateProgram;
import static org.lwjgl.opengl.GL20.glCreateShader;
import static org.lwjgl.opengl.GL20.glDeleteShader;
import static org.lwjgl.opengl.GL20.glDetachShader;
import static org.lwjgl.opengl.GL20.glGetProgramInfoLog;
import static org.lwjgl.opengl.GL20.glGetProgrami;
import static org.lwjgl.opengl.GL20.glGetShaderInfoLog;
import static org.lwjgl.opengl.GL20.glGetShaderi;
import static org.lwjgl.opengl.GL20.glLinkProgram;
import static org.lwjgl.opengl.GL20.glShaderSource;
import strb.core.Console;
import strb.gfx.Shader;

public class ShaderLoadingJob extends LoadingJob<Shader>{
	
	private final String VERTEX_SRC, FRAGMENT_SRC;
	private final String SHADER_NAME;
	
	public ShaderLoadingJob(String vertexSrc, String fragmentSrc, String shaderName) {
		VERTEX_SRC = vertexSrc;
		FRAGMENT_SRC = fragmentSrc;
		SHADER_NAME = shaderName;
	}
	
	@Override
	protected void perform() {
		int vsID = compile(VERTEX_SRC, GL_VERTEX_SHADER);
		int fsID = compile(FRAGMENT_SRC, GL_FRAGMENT_SHADER);
		int id = link(vsID, fsID);
		detach(id, vsID, fsID);
		result = new Shader(id);
	}
	
	private int compile(String shaderSrc, int shaderType) {
		int id = glCreateShader(shaderType);
		glShaderSource(id, shaderSrc);
		glCompileShader(id);
		int success = glGetShaderi(id, GL_COMPILE_STATUS);
		if (success == GL_FALSE) {
			int len = glGetShaderi(id, GL_INFO_LOG_LENGTH);
			System.out.println(glGetShaderInfoLog(id, len));
			Console.error("ERROR: " + SHADER_NAME + "["+ (shaderType==GL_VERTEX_SHADER?"vs":"fs") + "] failed to compile");
		}
		return id;
	}
	
	private int link(int vsID, int fsID) {
		int id = glCreateProgram();
		glAttachShader(id, vsID);
		glAttachShader(id, fsID);
		glLinkProgram(id);
		int success = glGetProgrami(id, GL_LINK_STATUS);
		if (success == GL_FALSE) {
			int len = glGetProgrami(id, GL_INFO_LOG_LENGTH);
			System.out.println("ERROR: linking a shaderProgram failed!");
			System.out.println(glGetProgramInfoLog(id, len));
			assert false : "";
		}
		return id;
	}
	private void detach(int id, int vsID, int fsID) {
		glDetachShader(id, vsID);
		glDeleteShader(vsID);
		glDetachShader(id, fsID);
		glDeleteShader(fsID);
	}
}
