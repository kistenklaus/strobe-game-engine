package strb.gfx;

import static org.lwjgl.opengl.GL15.glBindBuffer;
import static org.lwjgl.opengl.GL15.glBufferSubData;
import java.util.HashMap;
import org.joml.Matrix4f;
import strb.core.Console;
import static org.lwjgl.opengl.GL31.GL_UNIFORM_BUFFER;


public class UBO {
	
	private final int ID;
	
	private HashMap<String, Integer> uniformIndex = new HashMap<>();
	private int[] uniformOffsets;
	private String[] uniformRefrenceClassHashes;
	
	public UBO(int id, String[] uniformNames, int[] uniformOffsets, String[] uniformRefrenceClassHashes) {
		ID = id;
		for(int i=0;i<uniformNames.length;i++) {
			uniformIndex.put(uniformNames[i], i);
		}
		this.uniformOffsets = uniformOffsets;
		this.uniformRefrenceClassHashes = uniformRefrenceClassHashes;
	}
	
	public void uniformMat4Direct(String uniformName, Matrix4f mat4) {
		int uindex = uniformIndex.get(uniformName);
		if(!uniformRefrenceClassHashes[uindex].equals(Matrix4f.class.toString()))
			Console.error("invalid Type : Matrix4f");
		glBindBuffer(GL_UNIFORM_BUFFER, ID);
		float[] array = new float[16];
		mat4.get(array);
		glBufferSubData(GL_UNIFORM_BUFFER, uniformOffsets[uindex], array);
	}
	
	public void uniformMat4Later(String uniformName, Matrix4f mat4) {
		
	}
	
	public int getID() {
		return ID;
	}
	
	public class UBOContentMap {
		
	}
}
