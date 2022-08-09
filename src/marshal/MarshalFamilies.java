package marshal;

import java.util.HashMap;
import java.util.LinkedList;

class MarshalFamilies {
	
	private MarshalStaticArray<MarshalEntity> entities = new MarshalStaticArray<>();
	
	private MarshalStaticArray<MarshalFamily> families = new MarshalStaticArray<>();
	private HashMap<String, Integer> family_indicies = new HashMap<>();
	
	private LinkedList<FamilyChange> change_queue = new LinkedList<>();

	MarshalFamilies(MarshalStaticArray<MarshalEntity> entities){
		this.entities = entities;
	}
	
	void applyFamilyChanges() {
		while(change_queue.size()>0) {
			change_queue.removeFirst().apply();
		}
	}
	
	void enqueueEntityAdded(MarshalEntity entity) {
		change_queue.add(new EntityAdded(entity));
	}
	void enqueueEntityRemoved(MarshalEntity entity) {
		change_queue.add(new EntityRemoved(entity));
	}
	void enqueueComponentChange(MarshalComponent component) {
		change_queue.add(new ComponentModified(component));
	}
	
	boolean haveChanged() {
		return change_queue.size()>0;
	}
	
	private abstract class FamilyChange{
		abstract void apply();
	}
	
	MarshalFamily getFamily(MarshalFamily.MarshalFamilyIdentifier family_identifier) {
		Integer family_index = family_indicies.get(family_identifier.toString());
		MarshalFamily family;
		if(family_index == null) {
			family = new MarshalFamily(family_identifier);
			family_index = families.add(family);
			family_indicies.put(family_identifier.toString(), family_index);
			//check the family conditions against all entities;
			for(MarshalEntity entity : entities) {
				if(family.matches(entity)) {
					family.takeIn(entity);
				}
			}
			
		}else {
			family = families.get(family_index);
		}

		return family;
	}
	
	private final class EntityAdded extends FamilyChange{
		private final MarshalEntity entity;
		public EntityAdded(MarshalEntity entity) {
			this.entity = entity;
		}
		@Override
		void apply() {
			for(MarshalFamily family : families) {
				if(family.matches(entity)) {
					family.takeIn(entity);
				}else {
					family.exile(entity);
				}
			}
		}
	}
	private final class EntityRemoved extends FamilyChange{
		private final MarshalEntity entity;
		public EntityRemoved(MarshalEntity entity) {
			this.entity = entity;
		}
		void apply() {
			for(MarshalFamily family : families) {
				family.exile(entity);
			}
		}
	}
	private final class ComponentModified extends FamilyChange{
		private final MarshalComponent component;
		public ComponentModified(MarshalComponent component) {
			this.component = component;
		}
		
		@Override
		void apply() {
			for(MarshalFamily family : families) {
				if(family.dependsOnComponent(MarshalComponentType.get(component.getClass()))) {
					if(family.matches(component.root)) {
						family.takeIn(component.root);
					}else {
						family.exile(component.root);
					}
				}
			}
		}
		
	}
}
