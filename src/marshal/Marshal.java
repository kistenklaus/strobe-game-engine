package marshal;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.LinkedList;

public class Marshal {
	
	public static final int ENTITY_ADDED = 1;
	public static final int ENTITY_REMOVED = 2;
	public static final int COMPONENT_ADDED = 3;
	public static final int COMPONENT_REMOVED = 4;
	public static final int CONTEXT_INIT = 5;
	public static final int CONTEXT_DISPOSED = 6;
	public static final int CONTEXT_PRE_UPDATE = 7;
	public static final int CONTEXT_POST_UPDATE = 8;
	
	private static Marshal instance = null;

	static Marshal get() {
		if(instance == null)instance = new Marshal();
		return instance;
	}

	private LinkedList<MarshalContext.MarshalCallback> scheduled_entity_changes = new LinkedList<>();

	private MarshalStaticArray<MarshalEntity> entities = new MarshalStaticArray<>();
	private HashMap<MarshalEntity, Integer> entity_map = new HashMap<>();
	
	private ArrayList<MarshalContext> contexts = new ArrayList<>();
	
	private MarshalListener<MarshalComponent> component_added = new ComponentAddedListener();
	private MarshalListener<MarshalComponent> component_removed = new ComponentRemovedListener();
	
	private MarshalFamilies families = new MarshalFamilies(entities);
	private boolean running = false;
	
	private Marshal() { }
	
	public static void addContext(MarshalContext context) {
		get().contexts.add(context);
	}
	
	public static void start() {
		get().running = true;
		for(MarshalContext context : get().contexts) {
			if (context.threaded) context.start();
		}
	}
	
	public static void stop() {
		get().running = false;
		for(MarshalContext context : get().contexts) {
			context.stop();
		}
		Thread entity_dispose_thread = new Thread(()->get().entities.forEach(entity->entity.dispose()), "entity_dispose_thread");
		entity_dispose_thread.start();
		try {
			entity_dispose_thread.join();
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
	}

	static boolean hasEntity(MarshalEntity entity) {
		return get().has(entity);
	}
	
	private boolean has(MarshalEntity entity) {
		return entity_map.containsKey(entity);
	}
	
	static void scheduleEntityAdd(MarshalEntity entity) {
		if(!get().running && Thread.currentThread().getName().equals("marshal_thread")){
			get().addEntity(entity);
		}else{
			get().scheduled_entity_changes.add(()->get().addEntity(entity));
		}
	}

	private final void addEntity(MarshalEntity entity){
		System.out.println("enqueued");
		if(get().add(entity)) {
			boolean some_context_updating = false;
			for(MarshalContext context : get().contexts) {
				context.enqueueEvent(new MarshalEvent.ENTITY_ADDED(entity));
				//Families changed
				get().families.enqueueEntityAdded(entity);
				if(context.isUpdating()) some_context_updating = true;
			}
			if(!some_context_updating) get().families.applyFamilyChanges();
		}
	}

	private boolean add(MarshalEntity entity) {
		if(entity_map.containsKey(entity))return false;
		entity.component_added.add(component_added);
		entity.component_removed.add(component_added);
		int cache_index = entities.add(entity);
		entity.cache_index = cache_index;
		entity_map.put(entity, cache_index);
		return true;
	}
	
	static void scheduledEntityRemove(MarshalEntity entity) {
		if(!get().running && Thread.currentThread().getName().equals("marshal_thread")){
			get().removeEntity(entity);
		}else{
			get().scheduled_entity_changes.add(()->get().removeEntity(entity));
		}
	}

	private void removeEntity(MarshalEntity entity){
		if(get().remove(entity)) {
			boolean some_context_updating = false;
			for(MarshalContext context : get().contexts) {
				context.enqueueEvent(new MarshalEvent.ENTITY_REMOVED(entity));
				//Families changed
				get().families.enqueueEntityRemoved(entity);
				if(context.isUpdating()) some_context_updating = true;
			}
			entity.dispose();
			if(!some_context_updating) get().families.applyFamilyChanges();
		}
	}
	
	private boolean remove(MarshalEntity entity) {
		if(!entity_map.containsKey(entity)) return false;
		boolean successful = entities.remove(entity.cache_index)!=null;
		if(successful) {
			entity_map.remove(entity, entity.cache_index);
			entity.cache_index = -1;
			entities.remove(entity);//LINEAR_REMOVE
			entity.component_added.remove(component_added);
			entity.component_removed.remove(component_removed);
		}
		return successful;
	}

	static boolean entitiesHaveChanged(){
		return get().scheduled_entity_changes.size()>0;
	}
	
	public static LinkedList<MarshalEntity> getEntities(MarshalFamily.MarshalFamilyIdentifier family_identifier){
		return get().get(family_identifier);
	}

	public static MarshalStaticArray<MarshalEntity> getAllEntities(){
		return get().entities;
	}
	
	LinkedList<MarshalEntity> get(MarshalFamily.MarshalFamilyIdentifier family_identifier){
		return families.getFamily(family_identifier).getEntities();
	}
	
	public static MarshalEntity createEntity() {
		MarshalEntity entity =  new MarshalEntity();
		return entity;
	}
	
	static MarshalFamilies getFamilies() {
		return get().families;
	}

	static void waitForFamilySync(MarshalContext waiting_context) {
		boolean all_contexts_waiting = true;
		for(MarshalContext context : get().contexts) {
			if(!context.isWaiting()) {
				all_contexts_waiting = false;
				break;
			}
		}
		if(all_contexts_waiting) {
			for(MarshalContext context : get().contexts) {
				if(context.equals(waiting_context)) {
					//apply all entity adding/removing
					for(MarshalContext.MarshalCallback scheduled : get().scheduled_entity_changes){
						scheduled.callback();
					}
					get().scheduled_entity_changes.clear();
					//all contexts are waiting this is running on a random context_thread
					get().families.applyFamilyChanges();
					
					synchronized (context) {
						context.notify();
					}
				}
			}
		}else {
			synchronized (waiting_context) {
				try {
					waiting_context.wait();
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
			}
		}
	}
	
	
	public class ComponentAddedListener implements MarshalListener<MarshalComponent>{

		@Override
		public void envoke(MarshalComponent component) {
			for(MarshalContext context : get().contexts) {
				context.enqueueEvent(new MarshalEvent.COMPONENT_ADDED(component));
				//Family changes
				get().families.enqueueComponentChange(component);
			}
		}	
	}
	
	public class ComponentRemovedListener implements MarshalListener<MarshalComponent>{
		
		@Override
		public void envoke(MarshalComponent component) {
			for(MarshalContext context : get().contexts) {
				context.enqueueEvent(new MarshalEvent.COMPONENT_REMOVED(component));
				//Family changes
				get().families.enqueueComponentChange(component);
			}
		}
	}
}
