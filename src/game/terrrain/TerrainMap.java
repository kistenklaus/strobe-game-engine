package game.terrrain;

import java.util.ArrayList;
import java.util.List;

import engine.loading.Loader;

public class TerrainMap {
	private List<Terrain> terrains;
	public TerrainMap(int startX, int startZ, int widthX, int widthZ, Loader loader, TerrainTexurePack texturepack, TerrainTexture blendMap, String heightMap) {
		this.terrains = new ArrayList<>();
		for(int x = 0; x < widthX; x++) {
			for(int z = 0; z < widthZ; z++) {
				this.terrains.add(new Terrain(startX + x, startZ + z, loader, texturepack, blendMap, heightMap));
			}
		}
	}
	
	public List<Terrain> getTerrains() {
		return this.terrains;
	}
}
