package strb.event;


public interface Listener<T> {
	
	public void receive(Signal<T> signal, T object);
	
}
