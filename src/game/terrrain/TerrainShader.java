package game.terrrain;

import org.joml.Matrix4f;

import engine.gfx.shaders.Shader3D;

public class TerrainShader extends Shader3D{
	private static final String SOURCE_LOC = "./shaders/shaders3D/terrainShader/", VERTEX_FILE = "vertex.vs", FRAGMENT_FILE = "fragment.fs";
	
	private int location_bgTexture;
	private int location_rTexture;
	private int location_bTexture;
	private int location_gTexture;
	private int location_blendMap;
	
	public TerrainShader(Matrix4f projection) {
		super(SOURCE_LOC+VERTEX_FILE, SOURCE_LOC+FRAGMENT_FILE, projection);
	}
	@Override
	protected void getAllNewUniformLocations() {
		this.location_bgTexture = getUniformLoaction("bgTexture");
		this.location_rTexture = getUniformLoaction("rTexture");
		this.location_gTexture = getUniformLoaction("gTexture");
		this.location_bTexture = getUniformLoaction("bTexture");
		this.location_blendMap = getUniformLoaction("blendMap");
	}
	
	public void connectTexUnits() {
		super.loadInt(location_bgTexture, 0);
		super.loadInt(location_rTexture, 1);
		super.loadInt(location_gTexture, 2);
		super.loadInt(location_bTexture, 3);
		super.loadInt(location_blendMap, 4);
	}
	
	
	
}
