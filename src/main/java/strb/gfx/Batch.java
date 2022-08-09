package strb.gfx;

import static org.lwjgl.opengl.GL15.GL_ARRAY_BUFFER;
import static org.lwjgl.opengl.GL15.GL_ELEMENT_ARRAY_BUFFER;
import static org.lwjgl.opengl.GL15.glBindBuffer;
import static org.lwjgl.opengl.GL15.glBufferSubData;
import static org.lwjgl.opengl.GL30.glBindVertexArray;
import java.util.HashMap;
import strb.core.Console;
import strb.core.Engine;
import strb.multithreading.ExecutionJob;
import strb.util.Bag;

public class Batch extends VAO{

	private HashMap<VAO.Data, Integer> vaoIndex = new HashMap<>();
	private Bag<Integer> vertexOffsets = new Bag<>();
	private Bag<Integer> vaoVertexCount = new Bag<>();
	private VBO[] vbos;
	private int vertexCount = 0;
	private Engine engine;
	
	public Batch(int id, VBO[] vbos, Engine engine) {
		super(id, /*drawCount*/0);
		this.vbos = vbos;
		this.engine = engine;
	}
	
	public boolean insertDirect(VAO.Data vaoData) {
		if(vaoIndex.containsKey(vaoData))return false;
		if(!vaoData.isValid())Console.error("cant batch invalid vaoData");
		//inserting VertexArrays
		for(int i=0;i<vbos.length;i++) {
			int attribLength = vbos[i].getVertexLength();
			int vertexOffset = attribLength * vertexCount;
			float[] attrib = vaoData.getVertexAttribute(i);
			glBindBuffer(GL_ARRAY_BUFFER, vbos[i].getID());
			glBufferSubData(GL_ARRAY_BUFFER, vertexOffset * Float.BYTES, attrib);
		}
		
		//inserting Indicies:
		int[] shiftedIndicies = new int[vaoData.getIndicies().length];
		for(int i=0;i<shiftedIndicies.length;i++) {
			shiftedIndicies[i] = vaoData.getIndicies()[i] + vertexCount;
		}
		glBindVertexArray(ID);
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, drawCount*Integer.BYTES, shiftedIndicies);
		
		
		//caching the vertexOffset
		int cindex = vertexOffsets.add(vertexCount);
		vaoIndex.put(vaoData, cindex);
		vaoVertexCount.set(cindex, vaoData.getVertexCount());
		//increasing vertexCount & drawCount
		vertexCount += vaoData.getVertexCount();
		drawCount += shiftedIndicies.length;
		
		return true;
	}
	
	public void insertLater(VAO.Data vaoData) {
		engine.addGfxJob(new ExecutionJob(()->insertDirect(vaoData)), 0);
	}
	
	public void updateDirect(VAO.Data vaoData) {
		synchronized (vaoData) {
			int vindex = vaoIndex.get(vaoData);
			if(vaoVertexCount.get(vindex) != vaoData.getVertexCount())
				Console.error("IllegalOperation vaoData has changed its vertexCount!!!");
			int vertexOffset = vertexOffsets.get(vindex);
			for(int i=0;i<vaoData.getVertexAttributeCount();i++) {
				if(vaoData.isDirtyAttribute(i)) {
					int attribOffset = vertexOffset * vbos[i].getVertexLength();
					glBindBuffer(GL_ARRAY_BUFFER, vbos[i].getID());
					glBufferSubData(GL_ARRAY_BUFFER, attribOffset * Float.BYTES, vaoData.getVertexAttribute(i));
					vaoData.cleanAttribute(i);
				}
			}
		}
	}
	public void updateLater(VAO.Data vaoData) {
		engine.addGfxJob(new ExecutionJob(()->updateDirect(vaoData)), 0);
	}
	
}
