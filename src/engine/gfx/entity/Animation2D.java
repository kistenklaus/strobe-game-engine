package engine.gfx.entity;

import java.io.File;

import org.joml.Vector2f;

import engine.Clock;
import engine.gfx.models.Model2D;
import engine.gfx.models.RawModel2D;
import engine.gfx.textures.Texture;
import engine.loading.Loader;

public class Animation2D extends Entity2D{
	private Texture[] textures;
	private Clock clock;
	private int size, index;
	private double currFrameTime;
	private double frameTime;
	public Animation2D(String aniFolder,Vector2f pos, int level, float width, float height, float rotation, float scale, float aniSpeed, Clock clock, Loader loader) {
		super(new Model2D(RawModel2D.QUAD(width, height, loader), null), pos, level, rotation, scale);
		this.clock = clock;
		File[] sourceFolder = new File(aniFolder).listFiles();
		this.size = sourceFolder.length;
		this.frameTime = aniSpeed / size;
		this.index = 0;
		this.textures = new Texture[size];
		for(int i = 0; i < size; i++) {
			textures[i] = new Texture(loader.loadTextureID(sourceFolder[i].getPath()));
		}
		super.setTexture(textures[index]);
	}
	
	public void update() {
		currFrameTime += clock.getDeltaTime();
		if(currFrameTime > frameTime) {
			currFrameTime = 0;
			index++;
			if(index>=size)index=0;
		}
	}
	
	public Model2D getModel() {
		super.setTexture(textures[index]);
		return super.getModel();
	}

}
