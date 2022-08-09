package strb.logic.adapter;

import java.util.*;
import strb.core.Console;
import strb.core.Engine;
import strb.event.*;
import strb.gfx.*;
import strb.gfx.VAO.Data;
import strb.gfx.VAO.VertexLayout;
import strb.gfx.gfxJobs.*;
import strb.util.Bag;
import strb.multithreading.DataLock;

public class VAOAdapter {
	
	private Engine engine;
	
	private Bag<VAO> vaoCache = new Bag<>();
	private HashMap<String, Integer> vaoCacheIndex = new HashMap<>();
	private Bag<Batch> batchCache = new Bag<>();
	private HashMap<String, Integer> batchCacheIndex = new HashMap<>();
	
	private boolean inPremake = true;
	private List<DataLock<VAO>> pendingVAOs = new ArrayList<>();
	private List<DataLock<Batch>> pendingBatch = new ArrayList<>();
	
	public VAOAdapter(Engine engine) {
		this.engine = engine;
	}
	
	public VAO getVAO(String string) {
		return vaoCache.get(vaoCacheIndex.get(string));
	}
	
	public Batch getBatch(String string) {
		return batchCache.get(batchCacheIndex.get(string));
	}
	
	public DataLock<VAO> createVAO(String sid, Data meshData) {
		DataLock<VAO> completionLock = new DataLock<>();
		
		if(vaoCacheIndex.containsKey(sid)) {
			VAO vao = vaoCache.get(vaoCacheIndex.get(sid));
			completionLock.unlock(vao);
			return completionLock;
		}
		
		if(!meshData.isValid()) {
			Console.error("invalid MeshData [sid:"+sid+"]");
		}
		
		List<Signal<VBO>> vboCompleted = new ArrayList<>();
		List<VBOLoadingJob> vboJobs = new ArrayList<>();
		for(int i=0;i<meshData.getVertexAttributeCount();i++) {
			VBOLoadingJob vboJob = new VBOLoadingJob(meshData.getVertexAttribute(i), 
					meshData.getVertexAttributeLength(i), meshData.isAttributeDynamic(i));
			vboCompleted.add(vboJob.getCompletionLock().getUnlockSignal());
			vboJobs.add(vboJob);
		}
		Signal.join(vboCompleted).add((s, b)->{
			VBO[] vbos = new VBO[meshData.getVertexAttributeCount()];
			for(int i=0;i<vbos.length;i++) {
				vbos[i] = vboJobs.get(i).getResult();
			}
			VAOLoadingJob vaoJob = new VAOLoadingJob(meshData.getIndicies(), vbos);
			vaoJob.getCompletionLock().onUnlockData((_s, vao)->{
				int ci = vaoCache.add(vao);
				vaoCacheIndex.put(sid, ci);
				completionLock.unlock(vao);
			});
			engine.addGfxJob(vaoJob, 0);
		});
		
		for(VBOLoadingJob vboJob : vboJobs) {
			engine.addGfxJob(vboJob, 0);
		}
		
		if(inPremake)pendingVAOs.add(completionLock);
		
		return completionLock;
	}
	
	public DataLock<Batch> createBatch(String sid, int maxVertexCount, float averageVertexUsage, boolean dynamic, String...layout) {
		VertexLayout vaoLayout = new VertexLayout(layout);
		int maxDrawCount = (int) Math.ceil(averageVertexUsage * maxVertexCount);
		
		DataLock<Batch> completionLock = new DataLock<>();
		
		//create VBOs to match the Layout & the maxVertexCount
		List<Signal<VBO>> vboCompleted = new ArrayList<>(vaoLayout.getAttributeCount());
		List<VBOLoadingJob> vboJobs = new ArrayList<>(vaoLayout.getAttributeCount());
		for(int i=0;i<vaoLayout.getAttributeCount();i++) {
			VBOLoadingJob vboJob = new VBOLoadingJob(maxVertexCount, vaoLayout.getVertexAttribLengthOf(i), dynamic);
			vboJobs.add(vboJob);
			vboCompleted.add(vboJob.getCompletionLock().getUnlockSignal());
		}
		Signal.join(vboCompleted).add((s,b)->{
			//convert vboLocks -> VBO[]
			VBO[] vbos = new VBO[vaoLayout.getAttributeCount()];
			for(int i=0;i<vbos.length;i++)vbos[i] = vboJobs.get(i).getCompletionLock().getData();
			VAOLoadingJob vaoJob = new VAOLoadingJob(maxDrawCount, vbos);
			vaoJob.getCompletionLock().onUnlockData((_s, vao)->{
				//caching
				int vindex = vaoCache.add(vao);
				vaoCacheIndex.put(sid, vindex);
				Batch batch = new Batch(vao.getID(), vbos, engine);
				int bindex = batchCache.add(batch);
				batchCacheIndex.put(sid, bindex);
				//unlock
				completionLock.unlock(batch);
			});
			engine.addGfxJob(vaoJob, 0);
		});
		
		if(inPremake)pendingBatch.add(completionLock);
		
		for(VBOLoadingJob vboJob : vboJobs) {
			engine.addGfxJob(vboJob, 0);
		}
		
		return completionLock;
	}
	
	public void waitForPendingJobs() throws InterruptedException {
		for(DataLock<VAO> lock : pendingVAOs) {
			lock.waitForUnlock();
		}
		pendingVAOs.clear();
		for(DataLock<Batch> lock : pendingBatch) {
			lock.waitForUnlock();
		}
		pendingBatch.clear();
	}

	public void exitPremake() {
		inPremake = false;
		pendingVAOs = null;
		pendingBatch = null;
	}




	
}
