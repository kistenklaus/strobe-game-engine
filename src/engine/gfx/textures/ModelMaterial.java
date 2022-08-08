package engine.gfx.textures;

public class ModelMaterial {
	private int textureID;
	private float shineDamper, reflectivity;
	public ModelMaterial(int textureID) {
		super();
		this.textureID = textureID;
		this.shineDamper = Float.MAX_VALUE;
		this.reflectivity = 0;
	}
	public ModelMaterial(int textureID, float shineDamper, float reflectivity) {
		super();
		this.textureID = textureID;
		this.shineDamper = shineDamper;
		this.reflectivity = reflectivity;
	}
	public int getID() {
		return textureID;
	}
	public float getShineDamper() {
		return shineDamper;
	}
	public void setShineDamper(float shineDamper) {
		this.shineDamper = shineDamper;
	}
	public float getReflectivity() {
		return reflectivity;
	}
	public void setReflectivity(float reflectivity) {
		this.reflectivity = reflectivity;
	}
}
