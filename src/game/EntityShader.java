package game;

import org.joml.Matrix4f;

import engine.gfx.shaders.Shader3D;

public class EntityShader extends Shader3D{
	
	private static final String SOURCE_LOC = "./shaders/shaders3D/entityShader/", VERTEX_FILE = "vertex.vs", FRAGMENT_FILE = "fragment.fs";
	
	private int location_useFakeLighing;
	
	public EntityShader(Matrix4f projection) {
		super(SOURCE_LOC+VERTEX_FILE, SOURCE_LOC+FRAGMENT_FILE, projection);
	}

	@Override
	protected void getAllNewUniformLocations() {
		this.location_useFakeLighing = getUniformLoaction("useFakeLighing");
	}
	public void loadUseFakeLightingBoolean(boolean useFakeLight) {
		super.loadBoolean(location_useFakeLighing, useFakeLight);
	}
}
