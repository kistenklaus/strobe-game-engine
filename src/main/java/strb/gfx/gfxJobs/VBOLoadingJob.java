package strb.gfx.gfxJobs;

import static org.lwjgl.opengl.GL15.GL_ARRAY_BUFFER;
import static org.lwjgl.opengl.GL15.GL_DYNAMIC_DRAW;
import static org.lwjgl.opengl.GL15.GL_STATIC_DRAW;
import static org.lwjgl.opengl.GL15.glBindBuffer;
import static org.lwjgl.opengl.GL15.glBufferData;
import static org.lwjgl.opengl.GL15.glGenBuffers;
import strb.gfx.VBO;

public class VBOLoadingJob extends LoadingJob<VBO>{

	private final float[] CONTENT;
	private final int VERTEX_LENGTH;
	private final boolean ALLOCATE_ONLY;
	private final int MAX_VERTEX_COUNT;
	private final boolean DYNAMIC;
	
	public VBOLoadingJob(float[] content, int vertexLength, boolean dynamic) {
		CONTENT = content;
		VERTEX_LENGTH = vertexLength;
		ALLOCATE_ONLY = false;
		MAX_VERTEX_COUNT = -1;
		DYNAMIC = dynamic;
	}
	
	public VBOLoadingJob(int maxVertexCount, int vertexLength, boolean dynamic) {
		CONTENT = null;
		VERTEX_LENGTH = vertexLength;
		ALLOCATE_ONLY = true;
		MAX_VERTEX_COUNT = maxVertexCount;
		DYNAMIC = dynamic;
	}
	
	@Override
	protected void perform() {
		int usage = DYNAMIC ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
		int id = glGenBuffers();
		glBindBuffer(GL_ARRAY_BUFFER, id);
		if(ALLOCATE_ONLY) {
			glBufferData(GL_ARRAY_BUFFER, MAX_VERTEX_COUNT * VERTEX_LENGTH * Float.BYTES, usage);
		}else {
			glBufferData(GL_ARRAY_BUFFER, CONTENT, usage);
		}
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		
		result = new VBO(id, VERTEX_LENGTH);
	}
}
