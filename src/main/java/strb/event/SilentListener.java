package strb.event;


public class SilentListener<T> implements Listener<T>{

	private T data;
	
	@Override
	public void receive(Signal<T> signal, T data) {
		this.data = data;
	}
	
	public T getData() {
		return data;
	}
	
	
}
