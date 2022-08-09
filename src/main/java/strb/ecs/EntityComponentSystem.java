package strb.ecs;

import java.util.ArrayList;
import strb.event.Listener;
import strb.event.Signal;
import strb.multithreading.OperationQueue;

public class EntityComponentSystem {
	
	
	private Signal<Entity> entityAdded = new Signal<>();
	private Signal<Entity> entityRemoved = new Signal<>();
	
	private EntityManager entityManager = new EntityManager(new EntityListener());
	private EntitySystemManager systemManager = new EntitySystemManager(new EntitySystemListener());
	private FamilyManager familyManager = new FamilyManager(entityManager.getEntities());
	
	//dispatches when an entity is added/removed
	private Listener<Entity> componentChange = new ComponentListener();
	
	private OperationQueue<Entity> pendingComponentsOperations = new OperationQueue<>();
	
	private boolean updating = false;
	
	public EntityComponentSystem() {
		
	}
	
	public void addEntity(Entity entity) {
		boolean delayed = updating;
		entityManager.addEntity(entity, delayed);
	}
	public void removeEntity(Entity entity) {
		boolean delayed = updating;
		entityManager.removeEntity(entity, delayed);
	}
	public void addSystem(EntitySystem system) {
		systemManager.addSystem(system);
	}
	public void removeSystem(EntitySystem system) {
		systemManager.removeSystem(system);
	}
	public void update(float deltaTime) {
		if (updating) {
			throw new IllegalStateException("Cannot call update() on an Engine that is already updating.");
		}
		updating = true;
		//iterating over all Systems		
		for(EntitySystem system : systemManager.getSystemIterator()) {
			
			system.update(deltaTime);
			
			//checking for added/removed entities!
			entityManager.processPendingOperations();
			
			//checking for added/removed components!
			for(Entity entity : pendingComponentsOperations) {
				entity.notifyComponentChange();
			}
		}
		
		updating = false;
	}
	public ArrayList<Entity> getEntitiesFor(Family family){
		return familyManager.getEntitiesFor(family);
	}
	
	private void addEntityInternal(Entity entity) {
		entity.componentChange.add(componentChange);
		entity.pendingComponentChanges = pendingComponentsOperations;
		familyManager.updateFamilyMembership(entity);
	}
	
	private void removeEntityInternal(Entity entity) {
		familyManager.updateFamilyMembership(entity);
		
		entity.componentChange.remove(componentChange);
		entity.pendingComponentChanges = null;
	}
	
	private class ComponentListener implements Listener<Entity>{
		@Override
		public void receive(Signal<Entity> signal, Entity source) {
			familyManager.updateFamilyMembership(source);
		}
	}
	
	class EntitySystemListener {
		
		public void systemAdded(EntitySystem system) {
			system.onAdd(EntityComponentSystem.this);
		}
		
		public void systemRemoved(EntitySystem system) {
			system.onRemove(EntityComponentSystem.this);
		}
	}
	
	class EntityListener{
		
		public void entityAdded(Entity entity) {
			addEntityInternal(entity);
			entityAdded.dispatch(entity);
		}
		public void entityRemoved(Entity entity) {
			removeEntityInternal(entity);
			entityAdded.dispatch(entity);
		}
	}

	public void on(Event event, Listener<Entity> callback) {
		switch(event) {
			case ENTITYADD:
				entityAdded.add(callback);
				break;
			case ENTITYREMOVED:
				entityRemoved.add(callback);
				break;
		}
	}
	
	public static enum Event{
		ENTITYADD, ENTITYREMOVED;
	}
}
