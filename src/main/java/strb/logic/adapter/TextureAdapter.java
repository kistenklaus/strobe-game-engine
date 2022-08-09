package strb.logic.adapter;

import java.util.*;
import strb.core.Engine;
import strb.gfx.Texture;
import strb.gfx.gfxJobs.TextureLoadingJob;
import strb.io.ReadTextureFileJob;
import strb.multithreading.DataLock;
import strb.util.Bag;

public class TextureAdapter {
	
	private Engine engine;
	
	private Bag<Texture> textureCache = new Bag<>();
	private HashMap<String, Integer> textureCacheIndex = new HashMap<>();
	
	private boolean inPremake = true;
	private List<DataLock<Texture>> pendingTextures = new ArrayList<>();
	
	public TextureAdapter(Engine engine) {
		this.engine = engine;
	}
	
	public Texture getTexture(String filepath, boolean linearInterpolation) {
		String textureHash = "filepath:["+filepath+"];interpolation:["+linearInterpolation+"]";
		return textureCache.get(textureCacheIndex.get(textureHash));
	}
	
	public DataLock<Texture> loadTexture(String filepath, boolean linearInterpolation){
		DataLock<Texture> completionLock = new DataLock<Texture>();
		
		String textureHash = "filepath:["+filepath+"];interpolation:["+linearInterpolation+"]";
		
		if(textureCacheIndex.containsKey(textureHash)) { 
			Texture texture = textureCache.get(textureCacheIndex.get(textureHash));
			completionLock.unlock(texture);
			return completionLock;
		}
		
		ReadTextureFileJob readJob = new ReadTextureFileJob(filepath);
		readJob.getCompletionLock().onUnlockData((s, textureData)->{
			TextureLoadingJob loadingJob = new TextureLoadingJob(textureData, linearInterpolation);
			loadingJob.getCompletionLock().onUnlockData((_s, texture)->{
				textureData.free();
				int cacheIndex = textureCache.add(texture);
				textureCacheIndex.put(textureHash, cacheIndex);
				completionLock.unlock(texture);
			});
			engine.addGfxJob(loadingJob, 0);
		});
		
		
		if(inPremake)pendingTextures.add(completionLock);
		
		engine.addIOJob(readJob, 0);
		
		return completionLock;
	}
	
	public void waitForPendingJobs() throws InterruptedException {
		for(DataLock<Texture> lock : pendingTextures) {
			lock.waitForUnlock();
		}
		pendingTextures.clear();
		
	}

	public void exitPremake() {
		inPremake = false;
		pendingTextures = null;
	}


	
}
