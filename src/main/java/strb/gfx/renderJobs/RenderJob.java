package strb.gfx.renderJobs;

import strb.multithreading.Job;
import strb.multithreading.OperationQueue;

public abstract class RenderJob extends Job{
	
	protected OperationQueue<RenderJob> before = new OperationQueue<>();
	protected OperationQueue<RenderJob> after = new OperationQueue<>();
	
	public RenderJob() {
		
	}

	@Override
	protected void perform() {
		for(RenderJob job : before) {
			job.execute();
		}
		render();
		for(RenderJob job : after) {
			job.execute();
		}
	}
	
	protected abstract void render();
	
	public void sub(Type type, RenderJob subRender, int priority) {
		if(type == Type.BEFORE) {
			before.push(subRender, priority);
		}else if(type == Type.AFTER) {
			after.push(subRender, priority);
		}
	}
	
	public enum Type{
		BEFORE, AFTER
	}
}
