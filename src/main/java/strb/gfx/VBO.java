package strb.gfx;


public class VBO {
	
	private final int ID;
	private final int VERTEX_LENGTH;
	
	public VBO(int id, int vertexLength) {
		ID = id;
		VERTEX_LENGTH = vertexLength;
	}
	
	public int getID() {
		return ID;
	}
	
	public int getVertexLength() {
		return VERTEX_LENGTH;
	}
	
	public int hashCode() {
		return ID;
	}
	
}
