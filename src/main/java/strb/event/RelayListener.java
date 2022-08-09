package strb.event;


public class RelayListener<T> implements Listener<T>{
	
	private Signal<T> relay;
	public RelayListener(Signal<T> relay) {
		this.relay = relay;
	}
	
	public void receive(Signal<T> signal, T source) {
		relay.dispatch(source);
	}
	
}
