package strb.multithreading;

import strb.gfx.renderJobs.RenderJob;

public class JobHandler<T extends Job> {

	private OperationQueue<T> jobs;
	
	private OperationQueue<T> pending;
	
	public JobHandler() {
		jobs = new OperationQueue<T>();
		pending = new OperationQueue<T>();
	}
	
	public synchronized OperationQueue<T> getRenderJobs() {
		return jobs;
	}
	
	public synchronized void addJob(T job, int priority) {
		pending.push(job, priority);
	}
	
	public synchronized OperationQueue<T> updateJobs() {
		jobs.merge(pending);
		pending.clear();
		return jobs;
	}
	
	public synchronized void clearJobs() {
		jobs.clear();
	}

	public void removeJob(RenderJob renderJob) {
		jobs.remove(renderJob);
	}
	
	
}
