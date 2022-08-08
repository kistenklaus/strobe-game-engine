package rubiksCube;

import org.joml.Matrix4f;

import engine.gfx.shaders.Shader3D;

public class Shader3Dexample extends Shader3D{
	
	private static final String SOURCE_LOC = "./shaders/shaders3D/shader3Dexample/", VERTEX_FILE = "vertex.vs", FRAGMENT_FILE = "fragment.fs";
	
	public Shader3Dexample(Matrix4f projection) {
		super(SOURCE_LOC+VERTEX_FILE, SOURCE_LOC+FRAGMENT_FILE, projection);
	}

	@Override
	protected void getAllNewUniformLocations() {
		
	}

}
