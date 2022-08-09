package strb.ecs;

import java.util.ArrayList;
import java.util.HashSet;
import strb.ecs.EntityComponentSystem.EntityListener;
import strb.multithreading.OperationQueue;

class EntityManager {
	
	private ArrayList<Entity> entities = new ArrayList<>();
	private HashSet<Entity> entitySet = new HashSet<>();
	
	private EntityListener entityListener;
	private OperationQueue<Entity> pendingAdds = new OperationQueue<>();
	private OperationQueue<Entity> pendingRemoves = new OperationQueue<>(); 
	
	EntityManager(EntityListener entityListener) {
		this.entityListener = entityListener;
	}
	
	void addEntity(Entity entity, boolean delayed) {
		if(delayed) {
			pendingAdds.push(entity, 0);
		}else {
			addEntityInternal(entity);
		}
	}
	

	void removeEntity(Entity entity, boolean delayed) {
		if(delayed) {
			entity.scheduleForRemove = true;
			pendingRemoves.push(entity, 0);
		}else {
			removeEntityInternal(entity);
		}
	}
	
	void processPendingOperations() {
		if(!pendingAdds.isEmpty()) {
			for(Entity entity : pendingAdds) {
				addEntityInternal(entity);
			}
		}
		if(!pendingRemoves.isEmpty()) {
			for(Entity entity : pendingRemoves) {
				removeEntityInternal(entity);
			}
		}
	}

	private void addEntityInternal(Entity entity) {
		if(entitySet.contains(entity)) {
			throw new IllegalArgumentException("Entity is already registered" + entity);
		}
		entities.add(entity);
		entitySet.add(entity);
		entityListener.entityAdded(entity);
	}
	
	private void removeEntityInternal(Entity entity) {
		boolean removed = entitySet.remove(entity);
		
		if(removed) {
			entities.remove(entity);
			entityListener.entityRemoved(entity);
		}
		
	}

	ArrayList<Entity> getEntities() {
		return entities;
	}
	
	
}
