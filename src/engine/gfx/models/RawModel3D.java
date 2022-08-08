package engine.gfx.models;

public class RawModel3D {
	private int vaoID;
	private int vertexCount;
	public RawModel3D(int vaoID, int vertexCount) {
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
}
