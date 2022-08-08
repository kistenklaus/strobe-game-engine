package game;

import org.joml.Matrix3f;

import engine.gfx.shaders.Shader2D;

public class GameShader2D extends Shader2D{
	private static final String SOURCE_LOC = "./shaders/gameShader2D/", VERTEX_FILE ="vertex.vs", FRAGMENT_FILE ="fragment.fs";
	
	public GameShader2D(Matrix3f winMatrix) {
		super(SOURCE_LOC+VERTEX_FILE, SOURCE_LOC+FRAGMENT_FILE, winMatrix);
	}

	@Override
	protected void bindNewAttributes() {
		
		
	}

	@Override
	protected void getNewUniformLocation() {
		
		
	}
	
}
