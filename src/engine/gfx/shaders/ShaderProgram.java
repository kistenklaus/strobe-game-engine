package engine.gfx.shaders;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;

import org.joml.Matrix3f;
import org.joml.Matrix4f;
import org.joml.Vector3f;
import org.lwjgl.opengl.GL11;
import org.lwjgl.opengl.GL20;

public abstract class ShaderProgram {
	
	private int programID, vertexID, fragmentID;
	
	public ShaderProgram(String vertexFile, String fragmentFile) {
		this.vertexID = loadShader(vertexFile, GL20.GL_VERTEX_SHADER);
		this.fragmentID = loadShader(fragmentFile, GL20.GL_FRAGMENT_SHADER);
		this.programID = GL20.glCreateProgram();
		GL20.glAttachShader(programID, vertexID);
		GL20.glAttachShader(programID, fragmentID);
		bindAttributes();
		GL20.glLinkProgram(programID);
		GL20.glValidateProgram(programID);
		getAllUniformLocations();
	}
	
	protected abstract void bindAttributes();
	
	protected abstract void getAllUniformLocations();
	
	protected int getUniformLoaction(String uniformName) {
		int loc = GL20.glGetUniformLocation(programID, uniformName);
		if(loc == -1)System.err.println("Uniform: " + uniformName + " has not been found");
		return loc;
	}
	protected void loadBoolean(int loc, boolean value) {
		loadInt(loc, value?1:0);
	}
	protected void loadInt(int loc, int value) {
		GL20.glUniform1i(loc, value);
	}
	protected void loadFloat(int loc, float value) {
		GL20.glUniform1f(loc, value);
	}
	protected void loadVector3(int loc, Vector3f vec3) {
		loadVector3(loc, vec3.x, vec3.y, vec3.z);
	}
	protected void loadVector3(int loc, float x, float y, float z) {
		GL20.glUniform3f(loc, x, y, z);
	}
	protected void loadMatrix3(int loc, Matrix3f mat3) {
		GL20.glUniformMatrix3fv(loc, false, mat3.get(new float[9]));
	}
	protected void loadMatrix4(int loc, Matrix4f mat4) {
		GL20.glUniformMatrix4fv(loc, false, mat4.get(new float[16]));
	}
	protected void bindAttrib(int attrib, String attrib_name) {
		GL20.glBindAttribLocation(programID, attrib, attrib_name);
	}
	
	public void start() {
		GL20.glUseProgram(programID);
	}
	
	public void stop() {
		GL20.glUseProgram(0);
	}
	
	public void cleanUp() {
		stop();
		GL20.glDetachShader(programID, vertexID);
		GL20.glDetachShader(programID, fragmentID);
		GL20.glDeleteShader(vertexID);
		GL20.glDeleteShader(fragmentID);
		GL20.glDeleteProgram(programID);
	}
	
	private static int loadShader(String file, int type){
		StringBuilder shaderSource = new StringBuilder();
		try{
			BufferedReader reader = new BufferedReader(new FileReader(file));
			String line;
			while((line = reader.readLine())!=null){
				shaderSource.append(line).append("//\n");
			}
			reader.close();
		}catch(IOException e){
			e.printStackTrace();
			System.exit(-1);
		}
		int shaderID = GL20.glCreateShader(type);
		GL20.glShaderSource(shaderID, shaderSource);
		GL20.glCompileShader(shaderID);
		if(GL20.glGetShaderi(shaderID, GL20.GL_COMPILE_STATUS )== GL11.GL_FALSE){
			System.err.println("Could not compile shader! /n" + file + "\n____________________________________________________________________________________________" );
			System.out.println(GL20.glGetShaderInfoLog(shaderID, 500));
			System.exit(-1);
		}
		return shaderID;
	}
}