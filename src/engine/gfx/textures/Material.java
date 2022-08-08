package engine.gfx.textures;

public class Material {
	private Texture tex;
	private float shineDamper, reflectivity;
	private boolean hasTransparency;
	private boolean useFakeLighing;
	public Material(int textureID) {
		super();
		this.tex = new Texture(textureID);
		this.shineDamper = Float.MAX_VALUE;
		this.reflectivity = 0;
		this.shineDamper = 10;
		this.hasTransparency = false;
		this.useFakeLighing = false;
	}
	public int getID() {
		return tex.getID();
	}
	public Texture getTexture() {
		return this.tex;
	}
	public void setTexture(Texture tex) {
		this.tex = tex;
	}
	//MATERIAL VARIABLES:
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
