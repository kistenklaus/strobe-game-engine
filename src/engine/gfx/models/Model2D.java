package engine.gfx.models;

import engine.gfx.textures.Texture;
import engine.loading.Loader;

public class Model2D {
	private Texture texture;
	private RawModel2D model;
	public Model2D(RawModel2D model, Texture tex) {
		this.texture = tex;
		this.model = model;
	}
	public Model2D(String texturepath, Loader loader) {
		this.model = RawModel2D.IMAGE_FRAME(texturepath, loader);
		this.texture = new Texture(loader.loadTextureID(texturepath));
	}
	public RawModel2D getRawModel2D() {
		return this.model;
	}
	public Texture getTexture() {
		return texture;
	}
	public void setTexture(Texture texture) {
		this.texture = texture;
	}
	public int getTextureID() {
		return this.texture.getID();
	}
	public int getModelVAO() {
		return this.model.getVaoID();
	}
}
