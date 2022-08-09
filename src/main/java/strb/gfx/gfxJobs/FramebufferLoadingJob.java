package strb.gfx.gfxJobs;

import static org.lwjgl.opengl.GL11.GL_LINEAR;
import static org.lwjgl.opengl.GL11.GL_NEAREST;
import static org.lwjgl.opengl.GL11.GL_RGB;
import static org.lwjgl.opengl.GL11.GL_RGBA;
import static org.lwjgl.opengl.GL11.GL_TEXTURE_2D;
import static org.lwjgl.opengl.GL11.GL_TEXTURE_MAG_FILTER;
import static org.lwjgl.opengl.GL11.GL_TEXTURE_MIN_FILTER;
import static org.lwjgl.opengl.GL11.GL_UNSIGNED_BYTE;
import static org.lwjgl.opengl.GL11.glBindTexture;
import static org.lwjgl.opengl.GL11.glGenTextures;
import static org.lwjgl.opengl.GL11.glTexImage2D;
import static org.lwjgl.opengl.GL11.glTexParameteri;
import static org.lwjgl.opengl.GL30.GL_COLOR_ATTACHMENT0;
import static org.lwjgl.opengl.GL30.GL_DEPTH24_STENCIL8;
import static org.lwjgl.opengl.GL30.GL_DEPTH_STENCIL_ATTACHMENT;
import static org.lwjgl.opengl.GL30.GL_FRAMEBUFFER;
import static org.lwjgl.opengl.GL30.GL_RENDERBUFFER;
import static org.lwjgl.opengl.GL30.glBindFramebuffer;
import static org.lwjgl.opengl.GL30.glBindRenderbuffer;
import static org.lwjgl.opengl.GL30.glFramebufferRenderbuffer;
import static org.lwjgl.opengl.GL30.glFramebufferTexture2D;
import static org.lwjgl.opengl.GL30.glGenFramebuffers;
import static org.lwjgl.opengl.GL30.glGenRenderbuffers;
import static org.lwjgl.opengl.GL30.glRenderbufferStorage;
import static org.lwjgl.opengl.GL30.glRenderbufferStorageMultisample;
import static org.lwjgl.opengl.GL32.GL_TEXTURE_2D_MULTISAMPLE;
import static org.lwjgl.opengl.GL32.glTexImage2DMultisample;
import static org.lwjgl.system.MemoryUtil.NULL;
import strb.gfx.Framebuffer;
import strb.gfx.Framebuffer.Renderbuffer;
import strb.gfx.Framebuffer.TextureAttachment;

public class FramebufferLoadingJob extends LoadingJob<Framebuffer>{

	private final int WIDTH,HEIGHT,MULTISAMPLES;
	private final boolean LINEAR_INTERPOLATION,HASALPHA;
	
	public FramebufferLoadingJob(int width, int height, int multisamples, 
			boolean linearInterpolation, boolean hasAlpha) {
		WIDTH = width;
		HEIGHT = height;
		MULTISAMPLES = multisamples;
		LINEAR_INTERPOLATION = linearInterpolation;
		HASALPHA = hasAlpha;
	}
	
	@Override
	protected void perform() {
		final int ID = glGenFramebuffers();
		glBindFramebuffer(GL_FRAMEBUFFER, ID);
		//ColorAttachment:
		final int TEX_ID = glGenTextures();
		glBindTexture(GL_TEXTURE_2D, TEX_ID);
		final int textureBuffer = MULTISAMPLES > 1 ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
		final int format = HASALPHA ? GL_RGBA: GL_RGB;
		final int scaleMode = LINEAR_INTERPOLATION ? GL_LINEAR : GL_NEAREST;
		if(MULTISAMPLES>1) {
			glTexImage2DMultisample(textureBuffer, MULTISAMPLES, format, WIDTH, HEIGHT, true);
		}else {
			glTexImage2D(textureBuffer, 0, format, WIDTH, HEIGHT, 0, format, GL_UNSIGNED_BYTE, NULL);
		}
		glTexParameteri(textureBuffer, GL_TEXTURE_MIN_FILTER, scaleMode);
		glTexParameteri(textureBuffer, GL_TEXTURE_MAG_FILTER, scaleMode);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textureBuffer, TEX_ID, 0);
		TextureAttachment texAttachment = new TextureAttachment(TEX_ID, WIDTH, HEIGHT, MULTISAMPLES, HASALPHA);
		//Renderbuffer
		int RBO_ID = glGenRenderbuffers();
		glBindRenderbuffer(GL_RENDERBUFFER, RBO_ID);
		if(MULTISAMPLES>1) {
			glRenderbufferStorageMultisample(GL_RENDERBUFFER, MULTISAMPLES, GL_DEPTH24_STENCIL8, WIDTH, HEIGHT);
		}else {
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WIDTH, HEIGHT);
		}
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO_ID);
		Renderbuffer rboAttachment = new Renderbuffer(RBO_ID, WIDTH, HEIGHT, MULTISAMPLES);
		
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		
		result = new Framebuffer(ID, WIDTH, HEIGHT, texAttachment, rboAttachment);
	}
	
	
}
