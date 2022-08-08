package engine.gfx.models;

import engine.loading.Image;
import engine.loading.Loader;

public class RawModel2D {
	private int vaoID;
	private int vertexCount;
	public RawModel2D(int vaoID, int vertexCount) {
		super();
		this.vaoID = vaoID;
		this.vertexCount = vertexCount;
	}
	public int getVaoID() {
		return vaoID;
	}
	public int getVertexCount() {
		return vertexCount;
	}
	public static RawModel2D QUAD(float width, float height, Loader loader) {
		float u = width/2f, v = height/2f;
		return loader.loadTo2DVAO(new float[] {
				-u,v,
				u,v,
				u,-v,
				-u,-v,
		}, new float[] {
				0,0,
				0,1,
				1,1,
				1,0
		}, new int[] {
				0,3,2,
				2,1,0
		});
	}
	public static RawModel2D IMAGE_FRAME(String imagepath, Loader loader) {
		Image image = loader.loadImage(imagepath);
		return QUAD(image.getWidth(), image.getHeight(), loader);
	}
}
