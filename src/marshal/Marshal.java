package marshal;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.LinkedList;

public class Marshal {
	
	public static final int ENTITY_ADDED = 1;
	public static final int ENTITY_REMOVED = 2;
	public static final int COMPONENT_ADDED = 3;
	public static final int COMPONENT_REMOVED = 4;
	
	private static Marshal instance = null;

	static Marshal get() {
		if(instance == null)instance = new Marshal();
		return instance;
	}
	
	private MarshalStaticArray<MarshalEntity> entities = new MarshalStaticArray<>();
	private HashMap<MarshalEntity, Integer> entity_map = new HashMap<>();
	
	private ArrayList<MarshalContext> contexts = new ArrayList<>();
	
	private MarshalListener<MarshalComponent> component_added = new ComponentAddedListener();
	private MarshalListener<MarshalComponent> component_removed = new ComponentRemovedListener();
	
	private MarshalFamilies families = new MarshalFamilies(entities);
	
	private Marshal() {
		
	}
	
	public static void addContext(MarshalContext context) {
		get().contexts.add(context);
	}
	
	public static void start() {
		for(MarshalContext context : get().contexts) {
			context.start();
		}
	}
	
	public static void stop() {
		for(MarshalContext context : get().contexts) {
			context.stop();
		}
	}
	
	static boolean hasEntity(MarshalEntity entity) {
		return get().has(entity);
	}
	
	private boolean has(MarshalEntity entity) {
		return entity_map.containsKey(entity);
	}
	
	static void addEntity(MarshalEntity entity) {
		if(get().add(entity)) {
			boolean some_context_updating = false;
			for(MarshalContext context : get().contexts) {
				context.enqueueEvent(new MarshalEvent.ENTITY_ADDED(entity));
				//Families changed
				get().families.enqueueEntityAdded(entity);
				if(context.isUpdating()) some_context_updating = true;
			}
			if(!some_context_updating) get().families.applyFamilyChanges();
		}else {
			
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
	
	static void removeEntity(MarshalEntity entity) {
		if(get().remove(entity)) {
			boolean some_context_updating = false;
			for(MarshalContext context : get().contexts) {
				context.enqueueEvent(new MarshalEvent.ENTITY_REMOVED(entity));
				//Families changed
				get().families.enqueueEntityRemoved(entity);
				if(context.isUpdating()) some_context_updating = true;
			}
			if(!some_context_updating) get().families.applyFamilyChanges();
			
		}
	}
	
	boolean remove(MarshalEntity entity) {
		if(!entity_map.containsKey(entity)) return false;
		boolean successful = entities.remove(entity.cache_index);
		if(successful) {
			entity_map.remove(entity, entity.cache_index);
			entity.cache_index = -1;
			entities.remove(entity);//LINEAR_REMOVE
			entity.component_added.remove(component_added);
			entity.component_removed.remove(component_removed);
		}
		return successful;
	}
	
	public static LinkedList<MarshalEntity> getEntities(MarshalFamily.MarshalFamilyIdentifier family_identifier){
		return get().get(family_identifier);
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
				if(!context.equals(waiting_context)) {
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
