package game.terrrain;

import org.joml.Matrix4f;

import engine.gfx.shaders.Shader3D;

public class TerrainShader extends Shader3D{
	private static final String SOURCE_LOC = "./shaders/shaders3D/terrainShader/", VERTEX_FILE = "vertex.vs", FRAGMENT_FILE = "fragment.fs";
	
	public TerrainShader(Matrix4f projection) {
		super(SOURCE_LOC+VERTEX_FILE, SOURCE_LOC+FRAGMENT_FILE, projection);
	}
	@Override
	protected void getAllNewUniformLocations() {
		
	}
	
	
	
}
