package strb.gfx;

public class Framebuffer {
	
	private final int ID, WIDTH, HEIGHT;
	private final TextureAttachment TEXTURE_ATTACHMENT;
	private final Renderbuffer RENDERBUFFER;
	
	public Framebuffer(int id, int width, int height, 
			TextureAttachment textureAttachment, Renderbuffer renderbuffer) {
		ID = id;
		WIDTH = width;
		HEIGHT = height;
		TEXTURE_ATTACHMENT = textureAttachment;
		RENDERBUFFER = renderbuffer;
	}
	public int getID() {
		return ID;
	}
	
	public static class TextureAttachment {
		private final int TEX_ID,WIDTH,HEIGHT,MULTISAMPLES;
		private final boolean HASALPHA;
		public TextureAttachment(int texID, int width, int height, int multisamples, boolean hasAlpha) {
			TEX_ID = texID;
			WIDTH = width;
			HEIGHT = height;
			MULTISAMPLES = multisamples;
			HASALPHA = hasAlpha;
		}
	}
	
	public static class Renderbuffer{
		private final int ID, WIDTH,HEIGHT,MULTISAMPLES;
		public Renderbuffer(int id, int width, int height, int multisampled) {
			ID = id;
			WIDTH = width;
			HEIGHT = height;
			MULTISAMPLES = multisampled;
		}
	}
	
}
