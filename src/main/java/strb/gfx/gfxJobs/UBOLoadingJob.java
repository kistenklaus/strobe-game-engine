package strb.gfx.gfxJobs;

import static org.lwjgl.opengl.GL15.GL_DYNAMIC_DRAW;
import static org.lwjgl.opengl.GL15.glBindBuffer;
import static org.lwjgl.opengl.GL15.glBufferData;
import static org.lwjgl.opengl.GL15.glGenBuffers;
import static org.lwjgl.opengl.GL30.glBindBufferBase;
import static org.lwjgl.opengl.GL31.GL_UNIFORM_BUFFER;
import strb.gfx.UBO;

public class UBOLoadingJob extends LoadingJob<UBO>{
	
	private final int UBO_BINDING;
	private final int UBO_BYTE_SIZE;
	private final String[] UNIFORM_NAMES;
	private final int[] UNIFORM_OFFSETS;
	private final String[] UNIFORM_CLASS_REFRENCE_HASHES;
	
	public UBOLoadingJob(int binding, int uboByteSize, String[] uniformNames,
			int[] uniformOffsets, String[] uniformClassRefrenceHashes) {
		UBO_BYTE_SIZE = uboByteSize;
		UBO_BINDING = binding;
		UNIFORM_NAMES = uniformNames;
		UNIFORM_OFFSETS = uniformOffsets;
		UNIFORM_CLASS_REFRENCE_HASHES = uniformClassRefrenceHashes;
	}

	@Override
	protected void perform() {
		int id = glGenBuffers();
		glBindBuffer(GL_UNIFORM_BUFFER, id);
		glBufferData(GL_UNIFORM_BUFFER, UBO_BYTE_SIZE, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, UBO_BINDING, id);
		result = new UBO(id, UNIFORM_NAMES, UNIFORM_OFFSETS, UNIFORM_CLASS_REFRENCE_HASHES);
	}
}
