package strb.multithreading;

public abstract class CompletableJob<T> extends Job{
	
	protected DataLock<T> completionLock = new DataLock<>();
	
	protected T result;
	
	public void execute() {
		processing = true;
		perform();
		processing = false;
		completionLock.unlock(result);
	}
	
	public DataLock<T> getCompletionLock() {
		return completionLock;
	}
	
	public T getResult() {
		return result;
	}
	
	
}
