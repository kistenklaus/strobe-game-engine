package strb.gfx;

import static org.lwjgl.opengl.GL20.glGetUniformLocation;
import static org.lwjgl.opengl.GL20.glUniform1i;

public class Shader {
	
	private static int cacheIndex;
	
	private final int ID;
	private final int cacheID;
	
	public Shader(int id) {
		ID = id;
		cacheID = cacheIndex++;
	}

	public int getID() {
		return ID;
	}
	
	public int getCacheID() {
		return cacheID;
	}
	
	public void uniform1i(String uniformName, int uniformValue) {
		int loc = glGetUniformLocation(ID, uniformName);
		if(loc!=-1) {
			glUniform1i(loc, uniformValue);
		}
	}
}
