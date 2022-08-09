package marshal;

import java.util.ArrayList;
import java.util.LinkedList;

public abstract class MarshalContext implements Runnable{
	
	private boolean running = false;
	private boolean updating = false;
	private boolean waiting = false;
	
	private ArrayList<MarshalSystem> systems = new ArrayList<>();
	
	private MarshalSignal<MarshalEntity> entity_added = new MarshalSignal<>();
	private MarshalSignal<MarshalEntity> entity_removed = new MarshalSignal<>();
	
	private MarshalSignal<MarshalComponent> component_added = new MarshalSignal<>();
	private MarshalSignal<MarshalComponent> component_removed = new MarshalSignal<>();
	
	private LinkedList<MarshalEvent> event_queue = new LinkedList<>();
	
	private Thread contextThread;
	
	public MarshalContext() {
		contextThread = new Thread(this);
		component_added.add(component->{
			int component_index = MarshalComponentType.getIndex(component.getClass());
			MarshalSignal<MarshalComponent> signal = classified_component_added.get(component_index);
			if(signal != null) {
				signal.dispatch(component);
			}
		});
		component_removed.add(component->{
			int component_index = MarshalComponentType.getIndex(component.getClass());
			MarshalSignal<MarshalComponent> signal = classified_component_removed.get(component_index);
			if(signal != null) {
				signal.dispatch(component);
			}
		});
		
		entity_added.add(entity-> {
			for(MarshalComponent component : entity.components) {
				component_added.dispatch(component);
			}
		});
		entity_removed.add(entity->{
			for(MarshalComponent component : entity.components){
				component_removed.dispatch(component);
			}	
		});
	}
	
	public void start() {
		contextThread.start();
	}
	
	public void stop() {
		running = false;
	}
	
	public abstract void init();
	public abstract void update(float dt);
	public abstract void dispose();
	@Override
	public final void run() {
		running = true;
		init();
		
		waiting = true;
		Marshal.waitForFamilySync(this);
		waiting = false;
		
		long last = System.nanoTime();
		updating = true;
		while(running) {
			long curr = System.nanoTime();
			float dt = (curr-last) * 1e-9f;
			last = curr;
			for(MarshalSystem system : systems) {
				system.update(dt);
			}
			update(dt);
			
			/**
			 * Event Processing:
			 */
			synchronized(this) {
				while(event_queue.size() > 0) {
					MarshalEvent event = event_queue.removeFirst();
					event.poll(this);
				}
			}
			
			/**
			 * Family Synchronization:
			 * 
			 * -if any family has changed => wait for all contexts to wait
			 * after that open a new Thread to update all changed families
			 * when all families are updated resume all Contexts:
			 */
			if(Marshal.getFamilies().haveChanged()) {
				waiting = true;
				Marshal.waitForFamilySync(this);
				waiting = false;
			}
		}
		updating = false;
		dispose();
	}
	
	public final void add(MarshalSystem system) {
		systems.add(system);
	}
	public final void remove(MarshalSystem system) {
		systems.remove(system);
	}
	
	public final void add(MarshalEntity entity) {
		Marshal.addEntity(entity);
	}
	public final void remove(MarshalEntity entity) {
		Marshal.removeEntity(entity);
	}
	public final LinkedList<MarshalEntity> getEntities(MarshalFamily.MarshalFamilyIdentifier family_identifier) {
		return Marshal.getEntities(family_identifier);
	}
	
	public final void on(int event_type, MarshalListener<MarshalEntity> entity_listener) {
		
		switch(event_type) {
		case Marshal.ENTITY_ADDED:
			entity_added.add(entity_listener);
			break;
		case Marshal.ENTITY_REMOVED:
			entity_removed.add(entity_listener);
			break;
		default:
			throw new IllegalArgumentException("COMPONENT_ADDED or COMPONENT_REMOVED requires arguments on(type, component_class, listener)");
		}
	}
	
	MarshalStaticArray<MarshalSignal<MarshalComponent>> classified_component_added = new MarshalStaticArray<>();
	MarshalStaticArray<MarshalSignal<MarshalComponent>> classified_component_removed = new MarshalStaticArray<>();
	
	
	
	public final void on(int event_type, Class<? extends MarshalComponent> component_class, MarshalListener<MarshalComponent> component_listener) {
		int component_index;
		switch(event_type) {
		case Marshal.COMPONENT_ADDED:
			component_index = MarshalComponentType.getIndex(component_class);
			classified_component_added.computeIfNull(component_index, ()->new MarshalSignal<>()).add(component_listener);
			break;
		case Marshal.COMPONENT_REMOVED:
			component_index = MarshalComponentType.getIndex(component_class);
			classified_component_removed.computeIfNull(component_index, ()->new MarshalSignal<>()).add(component_listener);;
			break;
		default:
			throw new IllegalArgumentException("ENTITY_ADDED or ENTITY_REMOVED requires arguments on(type, listener)");
		}
	}
	
	final synchronized void enqueueEvent(MarshalEvent event) {
		event_queue.add(event);
	}
	final void dispatchEntityAdded(MarshalEntity entity) {
		entity_added.dispatch(entity);
	}
	final void dispatchEntityRemoved(MarshalEntity entity) {
		entity_removed.dispatch(entity);
	}
	final void dispatchComponentAdded(MarshalComponent component) {
		component_added.dispatch(component);
	}
	final void dispatchComponentRemoved(MarshalComponent component) {
		component_removed.dispatch(component);
	}
	final boolean isWaiting(){
		return waiting;
	}
	final boolean isUpdating() {
		return updating;
	}
}
