package engine.gfx.shaders;

import org.joml.Matrix3f;

public abstract class Shader2D extends ShaderProgram{
	
	private int location_window;
	private int location_transform;
	
	public Shader2D(String vertexFile, String fragmentFile, Matrix3f winMatrix) {
		super(vertexFile, fragmentFile);
		super.start();
		loadWindowMatrix(winMatrix);
		super.stop();
	}

	@Override
	protected void bindAttributes() {
		bindAttrib(0, "position");
		bindAttrib(1, "textureCoord");
		bindNewAttributes();
	}
	protected abstract void bindNewAttributes();

	@Override
	protected void getAllUniformLocations() {
		this.location_window = getUniformLoaction("window");
		this.location_transform = getUniformLoaction("transform");
		getNewUniformLocation();
	}
	protected abstract void getNewUniformLocation();
	
	public void loadWindowMatrix(Matrix3f windowMatrix) {
		loadMatrix3(location_window, windowMatrix);
	}
	public void loadTransformationMatrix(Matrix3f transform) {
		loadMatrix3(location_transform, transform);
	}
	
}
