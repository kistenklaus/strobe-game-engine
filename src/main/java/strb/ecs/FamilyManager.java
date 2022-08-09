package strb.ecs;

import java.util.*;
import strb.util.Bits;

class FamilyManager {
	
	private HashMap<Family, ArrayList<Entity>> families = new HashMap<>();
	
	private ArrayList<Entity> entities;
	
	FamilyManager(ArrayList<Entity> entities) {
		this.entities = entities;
	}
	
	ArrayList<Entity> getEntitiesFor(Family family){
		return requireFamily(family);
	}
	
	
	void updateFamilyMembership(Entity entity) {
		
		final Bits entityFamilyBits = entity.getFamilyBits();
		
		for(Family family : families.keySet()) {
			
			boolean belongsToFamily = entityFamilyBits.get(family.getIndex());
			boolean matches = family.matches(entity);
			
			if(belongsToFamily != matches) {
				final ArrayList<Entity> family_entities = families.get(family);
				
				if(matches) {
					entity.getFamilyBits().set(family.getIndex());
					family_entities.add(entity);
				}else {
					entity.getFamilyBits().clear(family.getIndex());
					family_entities.remove(entity);
				}
			}
		}
	}
	
	private ArrayList<Entity> requireFamily(Family family) {
		ArrayList<Entity> family_entities = families.get(family);
		
		if(family_entities == null) {
			family_entities = new ArrayList<>(16);
			families.put(family, family_entities);
			for(Entity entity : entities) {
				updateFamilyMembership(entity);
			}
		}
		return family_entities;
	}
	
	
}
