package strb.gfx.gfxJobs;

import static org.lwjgl.opengl.GL11.GL_LINEAR;
import static org.lwjgl.opengl.GL11.GL_LINEAR_MIPMAP_LINEAR;
import static org.lwjgl.opengl.GL11.GL_NEAREST;
import static org.lwjgl.opengl.GL11.GL_NEAREST_MIPMAP_LINEAR;
import static org.lwjgl.opengl.GL11.GL_REPEAT;
import static org.lwjgl.opengl.GL11.GL_RGB;
import static org.lwjgl.opengl.GL11.GL_RGBA;
import static org.lwjgl.opengl.GL11.GL_TEXTURE_2D;
import static org.lwjgl.opengl.GL11.GL_TEXTURE_MAG_FILTER;
import static org.lwjgl.opengl.GL11.GL_TEXTURE_MIN_FILTER;
import static org.lwjgl.opengl.GL11.GL_TEXTURE_WRAP_S;
import static org.lwjgl.opengl.GL11.GL_TEXTURE_WRAP_T;
import static org.lwjgl.opengl.GL11.GL_UNSIGNED_BYTE;
import static org.lwjgl.opengl.GL11.glBindTexture;
import static org.lwjgl.opengl.GL11.glGenTextures;
import static org.lwjgl.opengl.GL11.glTexImage2D;
import static org.lwjgl.opengl.GL11.glTexParameteri;
import static org.lwjgl.opengl.GL30.glGenerateMipmap;
import org.lwjgl.opengl.GL11;
import strb.core.Console;
import strb.gfx.Texture;

public class TextureLoadingJob extends LoadingJob<Texture>{

	private Texture.Data TEXTURE_DATA;
	private boolean LINEAR_INTERPOLATION;
	
	public TextureLoadingJob(Texture.Data textureData, boolean linearInterpolation) {
		TEXTURE_DATA = textureData;
		LINEAR_INTERPOLATION = linearInterpolation;
	}
	
	@Override
	protected void perform() {
		int id = glGenTextures();
		glBindTexture(GL_TEXTURE_2D, id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
				LINEAR_INTERPOLATION ? GL_LINEAR : GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
				LINEAR_INTERPOLATION ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_LINEAR);
		if(TEXTURE_DATA.getPixelData() != null) {
			switch(TEXTURE_DATA.getChannels()) {
				case 3:
					glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TEXTURE_DATA.getWidth(), TEXTURE_DATA.getHeight(),
							0, GL_RGB, GL_UNSIGNED_BYTE, TEXTURE_DATA.getPixelData());
					break;
				case 4:
					glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TEXTURE_DATA.getWidth(), TEXTURE_DATA.getHeight(),
							0, GL11.GL_RGBA, GL11.GL_UNSIGNED_BYTE, TEXTURE_DATA.getPixelData());
					break;
				default:
					Console.error("unknown number of channels : " + TEXTURE_DATA.getChannels());
			}
			glGenerateMipmap(GL_TEXTURE_2D);
		}else {
			Console.error("pixelData required!!!");
		}
		result = new Texture(id);
	}
}
