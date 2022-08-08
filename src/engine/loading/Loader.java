package engine.loading;

import java.io.File;
import java.nio.ByteBuffer;
import java.nio.FloatBuffer;
import java.nio.IntBuffer;
import java.util.ArrayList;
import java.util.List;

import org.joml.Matrix4f;
import org.lwjgl.BufferUtils;
import org.lwjgl.opengl.GL11;
import org.lwjgl.opengl.GL14;
import org.lwjgl.opengl.GL15;
import org.lwjgl.opengl.GL20;
import org.lwjgl.opengl.GL30;
import org.lwjgl.stb.STBImage;
import org.lwjgl.system.MemoryStack;

import engine.gfx.models.RawModel2D;
import engine.gfx.models.RawModel3D;
import engine.loading.objLoader.ModelData;
import engine.loading.objLoader.OBJFileLoader;

public class Loader {
	private List<Integer> vaos = new ArrayList<>();
	private List<Integer> vbos = new ArrayList<>();
	private List<Integer> texs = new ArrayList<>();
	private Matrix4f projection = new Matrix4f().identity();
	
	public RawModel3D loadTo3DVAO(float[] positions, float[] textures, float[] normals, int[] indices){
		int vaoID = createVAO();
		bindIndicesBuffer(indices);
		storeDataInAttributeList(0, 3, positions);
		storeDataInAttributeList(1, 2, textures);
		storeDataInAttributeList(2, 3, normals);
		unbindVAO();
		return new RawModel3D(vaoID, indices.length);
	}
	public RawModel2D loadTo2DVAO(float[] positions, float[] textures, int[] indices){
		int vaoID = createVAO();
		bindIndicesBuffer(indices);
		storeDataInAttributeList(0, 2, positions);
		storeDataInAttributeList(1, 2, textures);
		unbindVAO();
		return new RawModel2D(vaoID, indices.length);
	}
	
	
	private int createVAO() {
		int vaoID = GL30.glGenVertexArrays();
		vaos.add(vaoID);
		GL30.glBindVertexArray(vaoID);
		return vaoID;
	}
	private void storeDataInAttributeList(int attributeNumber, int dim, float[] data) {
		int vboID = GL15.glGenBuffers();
		vbos.add(vboID);
		GL15.glBindBuffer(GL15.GL_ARRAY_BUFFER, vboID);
		GL15.glBufferData(GL15.GL_ARRAY_BUFFER, storeDataInFloatBuffer(data), GL15.GL_STATIC_DRAW);
		GL20.glVertexAttribPointer(attributeNumber, dim, GL11.GL_FLOAT, false, 0, 0);
		GL15.glBindBuffer(vboID, 0);
	}
	
	public RawModel3D loadOBJModel(String objFile) {
		ModelData model = OBJFileLoader.loadOBJ(objFile);
		return loadTo3DVAO(model.getVertices(), model.getTextureCoords(), model.getNormals(), model.getIndices());
	}
	
	private void unbindVAO() {
		GL30.glBindVertexArray(0);
	}
	private void bindIndicesBuffer(int[] indices){
		int vboID = GL15.glGenBuffers();
		vbos.add(vboID);
		GL15.glBindBuffer(GL15.GL_ELEMENT_ARRAY_BUFFER, vboID);
		IntBuffer buffer = storeDataInIntBuffer(indices);
		GL15.glBufferData(GL15.GL_ELEMENT_ARRAY_BUFFER, buffer, GL15.GL_STATIC_DRAW);
	}
	private FloatBuffer storeDataInFloatBuffer(float[] data){
		FloatBuffer buffer = BufferUtils.createFloatBuffer(data.length);
		buffer.put(data);
		buffer.flip();
		return buffer;
	}
	private IntBuffer storeDataInIntBuffer(int[] data){
		IntBuffer buffer = BufferUtils.createIntBuffer(data.length);
		buffer.put(data);
		buffer.flip();
		return buffer;
	}
	public void cleanUp() {
		for(Integer vao: vaos) {
			GL30.glDeleteVertexArrays(vao);
		}
		for(Integer vbo: vbos) {
			GL15.glDeleteBuffers(vbo);
		}
		for(Integer tex: texs) {
			GL11.glDeleteTextures(tex);
		}
	}
	public void createProjMatrix(int winWidth, int winHeight, float FOV, float NEAR_PLANE, float FAR_PLANE) {
		float aspectRatio = (float) winWidth / winHeight;
		float y_scale = (float) ((1f / Math.tan(Math.toRadians(FOV /2f))) * aspectRatio);
		float x_scale =  y_scale / aspectRatio;
		float frustum_length = FAR_PLANE -NEAR_PLANE;
		this.projection = new Matrix4f();
		this.projection.m00(x_scale);
		this.projection.m11(y_scale);
		this.projection.m22( -( (FAR_PLANE + NEAR_PLANE) / frustum_length) );
		this.projection.m23(-1);
		this.projection.m32( -( (2* NEAR_PLANE*FAR_PLANE) / frustum_length ) );
		this.projection.m33(0);
	}
	public Matrix4f getProjMatrix() {
		return new Matrix4f(this.projection);
	}
	public int loadTextureID(String filename) {
		Image image = loadImage(filename);
		int id = GL11.glGenTextures();
		GL11.glBindTexture(GL11.GL_TEXTURE_2D, id);
		GL30.glGenerateMipmap(GL11.GL_TEXTURE_2D);
		GL11.glTexParameteri(GL11.GL_TEXTURE_2D, GL11.GL_TEXTURE_MIN_FILTER, GL11.GL_LINEAR_MIPMAP_LINEAR);
		GL11.glTexParameterf(GL11.GL_TEXTURE_2D, GL14.GL_TEXTURE_LOD_BIAS, 0);
		GL11.glTexParameterf(GL11.GL_TEXTURE_2D, GL11.GL_TEXTURE_MIN_FILTER, GL11.GL_LINEAR);
		GL11.glTexParameterf(GL11.GL_TEXTURE_2D, GL11.GL_TEXTURE_MAG_FILTER, GL11.GL_NEAREST);
		GL11.glTexImage2D(GL11.GL_TEXTURE_2D, 0, GL11.GL_RGBA, image.getWidth(), image.getHeight(), 0, GL11.GL_RGBA, GL11.GL_UNSIGNED_BYTE, image.getImage());
		GL11.glBindTexture(GL11.GL_TEXTURE_2D, id);
		return id;
	}
	public int loadTextureID(Image image) {
		int id = GL11.glGenTextures();
		GL11.glBindTexture(GL11.GL_TEXTURE_2D, id);
		GL30.glGenerateMipmap(GL11.GL_TEXTURE_2D);
		GL11.glTexParameteri(GL11.GL_TEXTURE_2D, GL11.GL_TEXTURE_MIN_FILTER, GL11.GL_LINEAR_MIPMAP_LINEAR);
		GL11.glTexParameterf(GL11.GL_TEXTURE_2D, GL14.GL_TEXTURE_LOD_BIAS, 0);
		GL11.glTexParameterf(GL11.GL_TEXTURE_2D, GL11.GL_TEXTURE_MIN_FILTER, GL11.GL_LINEAR);
		GL11.glTexParameterf(GL11.GL_TEXTURE_2D, GL11.GL_TEXTURE_MAG_FILTER, GL11.GL_NEAREST);
		GL11.glTexImage2D(GL11.GL_TEXTURE_2D, 0, GL11.GL_RGBA, image.getWidth(), image.getHeight(), 0, GL11.GL_RGBA, GL11.GL_UNSIGNED_BYTE, image.getImage());
		GL11.glBindTexture(GL11.GL_TEXTURE_2D, id);
		return id;
	}
	public Image loadImage(String path) {
		System.out.println(path);
		if(!new File(path).exists()) {System.err.println("Could not find the path: " + path);System.exit(-1);}
		ByteBuffer image;
		int width, height;
		try (MemoryStack stack = MemoryStack.stackPush()){
			IntBuffer comp = stack.mallocInt(1);
			IntBuffer w = stack.mallocInt(1);
			IntBuffer h = stack.mallocInt(1);
			image = STBImage.stbi_load(path, w, h, comp, 4);
			if (image == null) {
                throw new RuntimeException("Failed to load a texture file!"
                                           + System.lineSeparator() + STBImage.stbi_failure_reason());
            }
			width= w.get(0);
			height = w.get(0);
		}
		return new Image(image, width, height);
	}
}
