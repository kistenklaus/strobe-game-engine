package engine.gfx.shaders;

import org.joml.Matrix4f;
import org.joml.Vector3f;

import engine.gfx.entity.Light;
import engine.gfx.toolbox.Maths;

public abstract class Shader3D extends ShaderProgram{
	private int location_transformation;
	private int location_projection;
	private int location_view;
	private int location_lightPosition;
	private int location_lightColor;
	private int location_shineDamper;
	private int location_reflectivity;
	private int location_skyColor;
	
	public Shader3D(String vertexFile, String fragmentFile, Matrix4f projection) {
		super(vertexFile, fragmentFile);
		this.start();
		loadProjectionMatrix(projection);
		this.stop();
	}
	public Shader3D(String vertexFile, String fragmentFile, int winWidth, int winHeight, float FOV, float FAR_PLANE, float NEAR_PLANE) {
		super(vertexFile, fragmentFile);
		this.start();
		loadProjectionMatrix(Maths.createProjectionMatrix(winWidth, winHeight, FOV, FAR_PLANE, NEAR_PLANE));
		this.stop();
	}
	public void bindAttributes() {
		super.bindAttrib(0, "position");
		super.bindAttrib(1, "texCoords");
		super.bindAttrib(2, "normal");
	}
	protected void getAllUniformLocations() {
		this.location_transformation = getUniformLoaction("transformation");
		this.location_projection = getUniformLoaction("projection");
		this.location_view = getUniformLoaction("view");
		this.location_lightPosition = getUniformLoaction("lightPosition");
		this.location_lightColor = getUniformLoaction("lightColor");
		this.location_shineDamper = getUniformLoaction("shineDamper");
		this.location_reflectivity = getUniformLoaction("reflectivity");
		this.location_skyColor = getUniformLoaction("skyColor");
		this.getAllNewUniformLocations();
	}
	protected abstract void getAllNewUniformLocations();
	
	public void loadProjectionMatrix(Matrix4f projection) {
		loadMatrix4(location_projection, projection);
	}
	public void loadTransformationMatrix(Matrix4f transform) {
		loadMatrix4(location_transformation, transform);
	}
	public void loadViewMatrix(Vector3f cameraPosition, float pitch, float yaw, float roll) {
		loadMatrix4(location_view, Maths.createViewMatrix(cameraPosition, pitch, yaw, roll));
	}
	public void loadLight(Light light) {
		loadVector3(location_lightPosition, light.getPos());
		loadVector3(location_lightColor, light.getColor());
	}
	public void loadShineVariables(float shineDamper, float reflectivity) {
		super.loadFloat(location_shineDamper, shineDamper);
		super.loadFloat(location_reflectivity, reflectivity);
	}
	public void loadSkyColor(Vector3f skyColor) {
		super.loadVector3(location_skyColor, skyColor);
	}
}
