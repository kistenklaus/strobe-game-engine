package engine.loading;

import java.nio.ByteBuffer;

public class Image {
	private ByteBuffer image;
	private int width, height;
	
	public Image(ByteBuffer image, int width, int height) {
		super();
		this.image = image;
		this.width = width;
		this.height = height;
	}
	public ByteBuffer getImage() {
		return image;
	}
	public void setImage(ByteBuffer image) {
		this.image = image;
	}
	public int getWidth() {
		return width;
	}
	public void setWidth(int width) {
		this.width = width;
	}
	public int getHeight() {
		return height;
	}
	public void setHeight(int height) {
		this.height = height;
	}
	
}
