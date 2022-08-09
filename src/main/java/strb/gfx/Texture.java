package strb.gfx;

import static org.lwjgl.stb.STBImage.stbi_image_free;
import java.nio.ByteBuffer;

public class Texture {
	
	private final int ID;
	
	public Texture(int id) {
		ID = id;
	}
	
	public int getID() {
		return ID;
	}
	
	
	public static class Data{
		private final ByteBuffer PIXEL_DATA;
		private final int CHANNELS;
		private final int WIDTH, HEIGHT;
		public Data(ByteBuffer pixelData, int width, int height, int channels) {
			PIXEL_DATA = pixelData;
			WIDTH = width;
			HEIGHT = height;
			CHANNELS = channels;
		}
		public void free() {
			stbi_image_free(PIXEL_DATA);
		}
		public ByteBuffer getPixelData() {
			return PIXEL_DATA;
		}
		public int getChannels() {
			return CHANNELS;
		}
		public int getWidth() {
			return WIDTH;
		}
		public int getHeight() {
			return HEIGHT;
		}
	}
	
	public int hashCode() {
		return ID;
	}
	
}
