package game.terrrain;

import java.awt.image.BufferedImage;
import java.io.File;
import java.io.IOException;

import javax.imageio.ImageIO;

import engine.gfx.models.RawModel3D;
import engine.loading.Loader;

public class Terrain{
	private static final float SIZE = 800;
	private static final float MAX_HEIGHT = 40;
	private static final float MAX_PIXEL_COLOR = 256 * 256 * 256;
	private float x,z;
	private TerrainTexurePack texturepack;
	private TerrainTexture blendMap;
	private RawModel3D model;

	public Terrain(int gridX, int gridY, Loader loader, TerrainTexurePack texturepack, TerrainTexture blendMap, String heightMap) {
		this.x = gridX * SIZE;
		this.z = gridY * SIZE;
		this.texturepack = texturepack;
		this.blendMap = blendMap;
		this.model = generateTerrain(loader, heightMap);
	}
	
	private RawModel3D generateTerrain(Loader loader, String heightMap){
		
		BufferedImage image = null;
		try {
			image = ImageIO.read(new File(heightMap));
		} catch (IOException e) {
			e.printStackTrace();
		}
		int vertexCount = image.getHeight();
		
		int count = vertexCount * vertexCount;
		float[] vertices = new float[count * 3];
		float[] normals = new float[count * 3];
		float[] textureCoords = new float[count*2];
		int[] indices = new int[6*(vertexCount-1)*(vertexCount-1)];
		int vertexPointer = 0;
		for(int i=0;i<vertexCount;i++){
			for(int j=0;j<vertexCount;j++){
				vertices[vertexPointer*3] = (float)j/((float)vertexCount - 1) * SIZE;
				vertices[vertexPointer*3+1] = getHeight(j,i,image);
				vertices[vertexPointer*3+2] = (float)i/((float)vertexCount - 1) * SIZE;
				normals[vertexPointer*3] = 0;
				normals[vertexPointer*3+1] = 1;
				normals[vertexPointer*3+2] = 0;
				textureCoords[vertexPointer*2] = (float)j/((float)vertexCount - 1);
				textureCoords[vertexPointer*2+1] = (float)i/((float)vertexCount - 1);
				vertexPointer++;
			}
		}
		int pointer = 0;
		for(int gz=0;gz<vertexCount-1;gz++){
			for(int gx=0;gx<vertexCount-1;gx++){
				int topLeft = (gz*vertexCount)+gx;
				int topRight = topLeft + 1;
				int bottomLeft = ((gz+1)*vertexCount)+gx;
				int bottomRight = bottomLeft + 1;
				indices[pointer++] = topLeft;
				indices[pointer++] = bottomLeft;
				indices[pointer++] = topRight;
				indices[pointer++] = topRight;
				indices[pointer++] = bottomLeft;
				indices[pointer++] = bottomRight;
			}
		}
		return loader.loadTo3DVAO(vertices, textureCoords, normals, indices);
	}
	
	private float getHeight(int x, int z, BufferedImage image) {
		if(x<0 || x>=image.getHeight() || z<0 || z>=image.getHeight()) {
			return 0;
		}
		float height = image.getRGB(x, z);
		height += MAX_PIXEL_COLOR;
		height /= MAX_PIXEL_COLOR;
		height *= MAX_HEIGHT;
		return height;
	}

	public float getX() {
		return x;
	}

	public float getZ() {
		return z;
	}
	public TerrainTexurePack getTexturepack() {
		return texturepack;
	}
	public RawModel3D getModel() {
		return model;
	}
	public TerrainTexture getBlendMap() {
		return blendMap;
	}
	
}
