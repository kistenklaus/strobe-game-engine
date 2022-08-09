package marshal;

public abstract class MarshalComponent {
	MarshalEntity root = null;
	
	public MarshalEntity getRoot() {
		return root;
	}

	public void init(){ }

	public void dispose(){ }
}
