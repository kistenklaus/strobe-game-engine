package strb.io;

import static org.lwjgl.stb.STBImage.stbi_load;
import java.nio.ByteBuffer;
import java.nio.IntBuffer;
import org.lwjgl.BufferUtils;
import strb.core.Console;
import strb.gfx.Texture;

public class ReadTextureFileJob extends IOJob<Texture.Data>{
	
	private String FILEPATH;
	
	public ReadTextureFileJob(String filepath) {
		FILEPATH = filepath;
	}
	
	@Override
	protected void perform() {
		IntBuffer width = BufferUtils.createIntBuffer(1);
		IntBuffer height = BufferUtils.createIntBuffer(1);
		IntBuffer channels = BufferUtils.createIntBuffer(1);
		ByteBuffer pixelData = stbi_load(FILEPATH, width, height, channels,  0);
		if(pixelData == null)Console.error("could not load image :" + FILEPATH);
		
		result = new Texture.Data(pixelData, width.get(0), height.get(0), channels.get(0));
	}
	
}
