package engine.gfx;

import static org.lwjgl.opengl.GL11.GL_NEAREST;
import static org.lwjgl.opengl.GL11.GL_RGBA;
import static org.lwjgl.opengl.GL11.GL_TEXTURE_2D;
import static org.lwjgl.opengl.GL11.GL_TEXTURE_MAG_FILTER;
import static org.lwjgl.opengl.GL11.GL_TEXTURE_MIN_FILTER;
import static org.lwjgl.opengl.GL11.GL_UNSIGNED_BYTE;
import static org.lwjgl.opengl.GL11.glBindTexture;
import static org.lwjgl.opengl.GL11.glGenTextures;
import static org.lwjgl.opengl.GL11.glTexImage2D;
import static org.lwjgl.opengl.GL11.glTexParameterf;

import java.awt.image.BufferedImage;
import java.io.File;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.FloatBuffer;
import java.nio.IntBuffer;
import java.util.ArrayList;
import java.util.List;

import javax.imageio.ImageIO;

import org.joml.Matrix4f;
import org.lwjgl.BufferUtils;
import org.lwjgl.opengl.GL11;
import org.lwjgl.opengl.GL15;
import org.lwjgl.opengl.GL20;
import org.lwjgl.opengl.GL30;

import engine.gfx.models.RawModel;

public class Loader {
	private List<Integer> vaos = new ArrayList<>();
	private List<Integer> vbos = new ArrayList<>();
	private List<Integer> texs = new ArrayList<>();
	private Matrix4f projection = new Matrix4f().identity();
	public RawModel loadToVAO(float[] positions, float[] textures, float[] normals, int[] indices){
		int vaoID = createVAO();
		bindIndicesBuffer(indices);
		storeDataInAttributeList(0, 3, positions);
		storeDataInAttributeList(1, 2, textures);
		storeDataInAttributeList(2, 3, normals);
		unbindVAO();
		return new RawModel(vaoID, indices.length);
	}
	
	public int createVAO() {
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
	public int loadTexture(String filename) {
		BufferedImage bi;
		int tid = -1;
		try {
			bi = ImageIO.read(new File(filename));
			int width = bi.getWidth();
			int height = bi.getHeight();
			int[] pixels_raw = new int[width * height];
			pixels_raw = bi.getRGB(0, 0, width, height, null, 0, width);
			ByteBuffer pixels = BufferUtils.createByteBuffer(width* height *4);
			for(int x = 0; x < width; x++) {
				for(int y = 0; y < height; y++) {
					int pixel = pixels_raw[x*width + y];
					pixels.put((byte) ((pixel >> 16) & 0xFF));	//RED 
					pixels.put((byte) ((pixel >> 8) & 0xFF));	//GREEN
					pixels.put((byte) (pixel & 0xFF));			//BLUE
					pixels.put((byte) ((pixel >> 24) & 0xFF));	//ALPHA
				}
			}
			pixels.flip();
			tid = glGenTextures();
			glBindTexture(GL_TEXTURE_2D, tid);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
		}catch(IOException e) {e.printStackTrace();	}
		texs.add(tid);
		return tid;
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
}
