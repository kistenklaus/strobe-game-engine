package game.terrrain;

public class TerrainTexurePack {
	private TerrainTexture bgTex, rTex, bTex, gTex;
	public TerrainTexurePack(TerrainTexture bgTex, TerrainTexture rTex, TerrainTexture bTex, TerrainTexture gTex) {
		super();
		this.bgTex = bgTex;
		this.rTex = rTex;
		this.bTex = bTex;
		this.gTex = gTex;
	}
	public TerrainTexture getBgTex() {
		return bgTex;
	}
	public TerrainTexture getrTex() {
		return rTex;
	}
	public TerrainTexture getbTex() {
		return bTex;
	}
	public TerrainTexture getgTex() {
		return gTex;
	}
}
