package strb.multithreading;

public abstract class Job {
	
	private static int jobID;
	
	protected final int ID = jobID++;
	protected boolean processing = false;
	
	public void execute() {
		processing = true;
		perform();
		processing = false;
	}
	
	protected abstract void perform();
	
	
	public boolean isProcessing() {
		return processing;
	}
	public int getID() {
		return ID;
	}
}
