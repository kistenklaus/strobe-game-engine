package strb.ecs;


import java.lang.Iterable;
import java.util.*;
import strb.ecs.EntityComponentSystem.EntitySystemListener;

class EntitySystemManager {
	
	private EntitySystemListener systemsListener;
	
	private ArrayList<EntitySystem> systems = new ArrayList<>();
	private HashSet<Class<? extends EntitySystem>> systemClassSet = new HashSet<>();
	private Iterable<EntitySystem> systemIterable = new Iterable<EntitySystem>() {
		private int index = 0;
		private Iterator<EntitySystem> iterator = new Iterator<EntitySystem>() {
			@Override
			public EntitySystem next() {
				return systems.get(index++);
			}
			@Override
			public boolean hasNext() {
				return index<systems.size();
			}
		};
		@Override
		public Iterator<EntitySystem> iterator() {
			index = 0;
			return iterator;
		}
	};
	

	
	EntitySystemManager(EntitySystemListener entitySystemListener) {
		this.systemsListener = entitySystemListener;
	}

	void addSystem(EntitySystem system) {
		
		if(systemClassSet.contains(system.getClass())) {
			systems.remove(system);
		}else {
			systemClassSet.add(system.getClass());
		}
		
		int priority = system.getPriority();
		for(int i=0;i<systems.size();i++) {
			if(priority>systems.get(i).getPriority()) {
				systems.add(i, system);
				systemsListener.systemAdded(system);
				return;
			}
		}
		systemsListener.systemAdded(system);
		systems.add(system);
	}
	
	void removeSystem(EntitySystem system) {
		if(systemClassSet.contains(system.getClass())) {
			systems.remove(system);
			systemClassSet.remove(system.getClass());
			systemsListener.systemRemoved(system);
		}
	}
	
	Iterable<EntitySystem> getSystemIterator(){
		return systemIterable;
	}
	
}
