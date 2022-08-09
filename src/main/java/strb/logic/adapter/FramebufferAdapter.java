package strb.logic.adapter;

import java.util.*;
import strb.core.Engine;
import strb.gfx.Framebuffer;
import strb.gfx.gfxJobs.FramebufferLoadingJob;
import strb.multithreading.DataLock;
import strb.util.Bag;

public class FramebufferAdapter {
	
	private Engine engine;
	
	private Bag<Framebuffer> fboCache = new Bag<>();
	private HashMap<String, Integer> fboCacheIndex = new HashMap<>();
	
	private boolean inPremake = true;
	private List<DataLock<Framebuffer>> pendingFBOs = new ArrayList<>();
	
	public FramebufferAdapter(Engine engine) {
		this.engine = engine;
	}
	
	public Framebuffer get(String sid) {
		Integer findex = fboCacheIndex.get(sid);
		if(findex == null)return null;
		return fboCache.get(findex);
	}
	
	public DataLock<Framebuffer> createFramebuffer(String sid, int width, int height,
			int multisamples, boolean linearInterpolation, boolean hasAlpha) {
		
		if(fboCacheIndex.containsKey(sid)) {
			Framebuffer fbo = fboCache.get(fboCacheIndex.get(sid));
			DataLock<Framebuffer> lock = new DataLock<Framebuffer>();
			lock.unlock(fbo);
			return lock;
		}
		
		FramebufferLoadingJob fboJob = new FramebufferLoadingJob(width, height, multisamples, linearInterpolation, hasAlpha);
		
		fboJob.getCompletionLock().onUnlockData((s,fbo)->{
			int findex = fboCache.add(fbo);
			fboCacheIndex.put(sid, findex);
		});
		engine.addGfxJob(fboJob, 0);
		
		if(inPremake)pendingFBOs.add(fboJob.getCompletionLock());
		
		return fboJob.getCompletionLock();
	}
	
	public void waitForPendingJobs() throws InterruptedException{
		for(DataLock<Framebuffer> lock : pendingFBOs) {
			lock.waitForUnlock();
		}
		pendingFBOs.clear();
	}
	
	public void exitPremake() {
		inPremake = false;
		pendingFBOs = null;
	}


	
}
