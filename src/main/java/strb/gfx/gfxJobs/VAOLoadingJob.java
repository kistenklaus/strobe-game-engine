package strb.gfx.gfxJobs;

import static org.lwjgl.opengl.GL11.GL_FLOAT;
import static org.lwjgl.opengl.GL15.GL_ARRAY_BUFFER;
import static org.lwjgl.opengl.GL15.GL_DYNAMIC_DRAW;
import static org.lwjgl.opengl.GL15.GL_ELEMENT_ARRAY_BUFFER;
import static org.lwjgl.opengl.GL15.GL_STATIC_DRAW;
import static org.lwjgl.opengl.GL15.glBindBuffer;
import static org.lwjgl.opengl.GL15.glBufferData;
import static org.lwjgl.opengl.GL15.glGenBuffers;
import static org.lwjgl.opengl.GL20.glEnableVertexAttribArray;
import static org.lwjgl.opengl.GL20.glVertexAttribPointer;
import static org.lwjgl.opengl.GL30.glBindVertexArray;
import static org.lwjgl.opengl.GL30.glGenVertexArrays;
import strb.gfx.VAO;
import strb.gfx.VBO;

public class VAOLoadingJob extends LoadingJob<VAO>{
	
	private final VBO[] VBOs;
	private final int[] INDICIES;
	private final boolean INDICIES_ALLOCATE_ONLY;
	private final int MAX_DRAWCOUNT;
	public boolean DYNAMIC_INDICIES = false;
	
	public VAOLoadingJob(int[] indicies, VBO... vbos) {
		INDICIES = indicies;
		INDICIES_ALLOCATE_ONLY = false;
		MAX_DRAWCOUNT = -1;
		VBOs = vbos;
	}
	public VAOLoadingJob(int maxDrawCount, VBO...vbos) {
		INDICIES = null;
		INDICIES_ALLOCATE_ONLY = true;
		MAX_DRAWCOUNT = maxDrawCount;
		VBOs = vbos;
	}
	
	@Override
	protected void perform() {
		int id = glGenVertexArrays();
		glBindVertexArray(id);
		for(int i=0;i<VBOs.length;i++) {
			VBO vbo = VBOs[i];
			glBindBuffer(GL_ARRAY_BUFFER, vbo.getID());
			glEnableVertexAttribArray(i);
			glVertexAttribPointer(i, vbo.getVertexLength(), GL_FLOAT, false, 0, 0);
		}
		int ebo = glGenBuffers();
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		int drawCount;
		if(INDICIES_ALLOCATE_ONLY) {
			drawCount = MAX_DRAWCOUNT/Integer.BYTES;
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_DRAWCOUNT*Integer.BYTES, DYNAMIC_INDICIES ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);
		}else {
			drawCount = INDICIES.length;
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, INDICIES, DYNAMIC_INDICIES ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);
		}
		glBindVertexArray(0);
		result = new VAO(id, drawCount);
	}
}
