package strb.logic.adapter;

import java.util.*;
import org.joml.*;
import strb.core.Console;
import strb.core.Engine;
import strb.gfx.UBO;
import strb.gfx.gfxJobs.UBOLoadingJob;
import strb.multithreading.DataLock;
import strb.util.Bag;

public class UBOAdapter {

	private Engine engine;
	
	private int uboBindingIndex = 0;
	private HashMap<String, Integer> uboCacheIndex = new HashMap<>();
	private Bag<UBO> uboCache = new Bag<>();
	
	private boolean inPremake = true;
	private List<DataLock<UBO>> pendingUBOs = new ArrayList<>();
	
	public UBOAdapter(Engine engine) {
		this.engine = engine;
	}
	
	public UBO get(String name) {
		return uboCache.get(uboCacheIndex.get(name));
	}
	
	public DataLock<UBO> create(String name, String...definition) {
		
		if(uboCacheIndex.containsKey(name)) {
			UBO ubo = uboCache.get(uboCacheIndex.get(name));
			DataLock<UBO> found = new DataLock<>();
			found.unlock(ubo);
			return found;
		}
		
		String[] uniformNames = new String[definition.length];
		String[] uniformRefrenceClasses = new String[definition.length];
		int[] uniformOffsets = new int[definition.length];
		int uboByteSize = 0;
		for(int i=0;i<definition.length;i++) {
			String[] vardef = definition[i].split(" ");
			if(vardef.length != 2)Console.error("invalid format!");
			String varType = vardef[0];
			String varName = vardef[1];
			String varRefrenceClass = null;
			int baseAlignment = -1;
			int count = -1;
			switch(varType) {
				case "mat4":
					varRefrenceClass = Matrix4f.class.toString();
					baseAlignment = 16;
					count = 4;
					break;
				case "vec4":
					varRefrenceClass = Vector4f.class.toString();
					baseAlignment = 16;
					count = 1;
					break;
				case "vec3":
					varRefrenceClass = Vector3f.class.toString();
					baseAlignment = 16;
					count = 1;
					break;
				case "vec2":
					varRefrenceClass = Vector2f.class.toString();
					baseAlignment = 8;
					count = 1;
					break;
				default :
					Console.error("invalid or undefined Type :[" + varType+"]");
			}
			if(varRefrenceClass == null)Console.error("varRefrenceClass is undefined!");
			if(baseAlignment == -1)Console.error("baseAlignment is undefined!");
			if(count == -1)Console.error("count is undefined!");
			
			int aligned_offset = (int) (java.lang.Math.ceil(uboByteSize/baseAlignment) * baseAlignment);
			
			uboByteSize = aligned_offset + baseAlignment * count;
			
			uniformOffsets[i] = aligned_offset;
			uniformRefrenceClasses[i] = varRefrenceClass;
			uniformNames[i] = varName;
		}
		int binding = uboBindingIndex++;
		
		UBOLoadingJob uboJob = new UBOLoadingJob(binding, uboByteSize,
				uniformNames, uniformOffsets,
				uniformRefrenceClasses);
		
		uboJob.getCompletionLock().onUnlockData((s,ubo)->{
			uboCache.set(binding, ubo);
			uboCacheIndex.put(name, binding);
		});
		
		if(inPremake)pendingUBOs.add(uboJob.getCompletionLock());
		
		engine.addGfxJob(uboJob, 0);
		
		return uboJob.getCompletionLock();
	}

	public void waitForPendingJobs() throws InterruptedException {
		for(DataLock<UBO> uboJob : pendingUBOs) {
			uboJob.waitForUnlock();
		}
		pendingUBOs.clear();
	}

	public void exitPremake() {
		inPremake = false;
		pendingUBOs = null;
	}
	
}
