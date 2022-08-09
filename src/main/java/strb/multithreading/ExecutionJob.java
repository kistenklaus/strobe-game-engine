package strb.multithreading;

public class ExecutionJob extends Job{
	
	private Methode methode;
	
	public ExecutionJob(Methode methode) {
		this.methode = methode;
	}

	@Override
	protected void perform() {
		methode.invoke();
	}
	
	public interface Methode{
		void invoke();
	}
	
}
