package engine.gfx.models;

import engine.gfx.textures.Material;

public class Model3D {
	private RawModel3D rawmodel;
	private Material material;
	public Model3D(RawModel3D rawmodel, Material material) {
		super();
		this.rawmodel = rawmodel;
		this.material = material;
	}
	public RawModel3D getRawModel() {
		return rawmodel;
	}
	/**
	 * @deprecated
	 * @return
	 */
	public Material getTexture() {
		return material;
	}
	public Material getMaterial() {
		return material;
	}
}
