package strb.logic.adapter;

import java.util.*;
import strb.core.Engine;
import strb.event.Signal;
import strb.gfx.Shader;
import strb.gfx.gfxJobs.ShaderLoadingJob;
import strb.io.ReadFileJob;
import strb.multithreading.DataLock;
import strb.util.Bag;

public class ShaderAdapter {
	
	private Engine engine;
	
	private Bag<Shader> shaderCache = new Bag<>();
	private HashMap<String, Integer> shaderCacheIndex = new HashMap<>();
	
	private boolean inPremake = true;
	private List<DataLock<Shader>> pendingShaders = new ArrayList<>();
	
	public ShaderAdapter(Engine engine) {
		this.engine = engine;
	}
	
	public Shader getShader(String filepath) {
		return shaderCache.get(shaderCacheIndex.get(filepath));
	}
	
	public DataLock<Shader> compileShader(String filepath){
		DataLock<Shader> completionLock = new DataLock<>();
		if(shaderCacheIndex.containsKey(filepath)) {
			Shader shader = shaderCache.get(shaderCacheIndex.get(filepath));
			completionLock.unlock(shader);
			return completionLock;
		}
		
		//creating io jobs
		ReadFileJob readVertex = new ReadFileJob(filepath + ".vs");
		ReadFileJob readFragment = new ReadFileJob(filepath + ".fs");
		
		Signal.join(readVertex.getCompletionLock().getUnlockSignal(),
			readFragment.getCompletionLock().getUnlockSignal())
			.add((s,b)->{
				String[] split = filepath.split("/");
				ShaderLoadingJob shaderJob = new ShaderLoadingJob(readVertex.getResult(),
						readFragment.getResult(), split[split.length-1]);
				shaderJob.getCompletionLock().onUnlockData((_s,shader)->{
					int ci = shaderCache.add(shader);
					shaderCacheIndex.put(filepath, ci);
					completionLock.unlock(shader);
				});
				engine.addGfxJob(shaderJob, 0);
			});
		
		if(inPremake)pendingShaders.add(completionLock);
		//adding io jobs
		engine.addIOJob(readVertex, 0);
		engine.addIOJob(readFragment, 0);
		
		
		return completionLock;
	}
	
	public void waitForPendingJobs() throws InterruptedException {
		for(DataLock<Shader> locks : pendingShaders) {
			locks.waitForUnlock();
		}
		pendingShaders.clear();
		
	}

	public void exitPremak() {
		inPremake = false;
		pendingShaders = null;
	}

}
