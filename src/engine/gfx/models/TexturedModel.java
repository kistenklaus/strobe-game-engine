package engine.gfx.models;

import engine.gfx.textures.ModelMaterial;

public class TexturedModel {
	private RawModel rawmodel;
	private ModelMaterial material;
	public TexturedModel(RawModel rawmodel, ModelMaterial material) {
		super();
		this.rawmodel = rawmodel;
		this.material = material;
	}
	public RawModel getRawModel() {
		return rawmodel;
	}
	/**
	 * @deprecated
	 * @return
	 */
	public ModelMaterial getTexture() {
		return material;
	}
	public ModelMaterial getMaterial() {
		return material;
	}
}
