package engine.gfx.textures;

public class ModelMaterial {
	private int textureID;
	private float shineDamper, reflectivity;
	private boolean hasTransparency = false;
	private boolean useFakeLighing = false;
	public ModelMaterial(int textureID) {
		super();
		this.textureID = textureID;
		this.shineDamper = Float.MAX_VALUE;
		this.reflectivity = 0;
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
	public boolean hasTransparency() {
		return hasTransparency;
	}
	public void setHasTransparency(boolean hasTransparency) {
		this.hasTransparency = hasTransparency;
	}
	public boolean hasFakeLighing() {
		return useFakeLighing;
	}
	public void setFakeLighing(boolean useFakeLighing) {
		this.useFakeLighing = useFakeLighing;
	}
	
}
