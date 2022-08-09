package strb.ecs;

import java.util.ArrayList;
import strb.event.Signal;
import strb.multithreading.OperationQueue;
import strb.util.*;

public class Entity {
	
	private Bag<Component> components;
	private ArrayList<Component> componentsArray;
	private Bits componentBits;
	private Bits familyBits;
	
	Signal<Entity> componentChange = new Signal<>();
	OperationQueue<Entity> pendingComponentChanges;
	
	boolean scheduleForRemove = false;
	
	public Entity() {
		components = new Bag<>();
		componentsArray = new ArrayList<>();
		componentBits = new Bits();
		familyBits = new Bits();
	}
	
	void notifyComponentChange() {
		componentChange.dispatch(this);
	}
	public void add(Component component) {
		addInternal(component);
		
		//entity is added to the ECS
		if(pendingComponentChanges != null) {
			pendingComponentChanges.push(this, 0);
		}else {
			//entity has not been added to the ECS
			componentChange.dispatch(this);
		}
	}
	
	public Component remove(Class<? extends Component> component_class) {
		int componentTypeIndex = ComponentType.getIndexFor(component_class);
		
		if(components.isIndexWithinBounds(componentTypeIndex)) {
			Component removedComponent = components.get(componentTypeIndex);
			
			if(removedComponent != null && removeInternal(component_class) != null) {
				//successfully removed
				if(pendingComponentChanges != null) {
					pendingComponentChanges.push(this, 0);
				}else{
					componentChange.dispatch(this);
				}
			}
			
			return removedComponent;
		}
		return null;
	}
	
	public <T extends Component> T getComponent(Class<T> component_class) {
		return getComponent(ComponentType.getFor(component_class));
	}
	
	@SuppressWarnings("unchecked")
	<T extends Component> T getComponent(ComponentType componentType) {
		int componentTypeIndex = componentType.getIndex();
		if(componentTypeIndex < components.getCapacity()) {
			return (T)components.get(componentTypeIndex);
		}else {
			return null;
		}
	}
	
	public boolean hasComponent(Class<? extends Component> component_class) {
		return hasComponent(ComponentType.getFor(component_class));
	}
	
	boolean hasComponent(ComponentType componentType) {
		return componentBits.get(componentType.getIndex());
	}
	
	Bits getComponentBits() {
		return componentBits;
	}
	Bits getFamilyBits() {
		return familyBits;
	}
	
	private void addInternal(Component component) {
		Class<? extends Component> component_class = component.getClass();
		//remove previous Component of type componentClass
		
		int componentTypeIndex = ComponentType.getIndexFor(component_class);
		components.set(componentTypeIndex, component);
		componentsArray.add(component);
		componentBits.set(componentTypeIndex);
	}
	private Component removeInternal(Class<? extends Component> component_class) {
		int componentTypeIndex = ComponentType.getIndexFor(component_class);
		Component removedComponent = components.get(componentTypeIndex);
		if(removedComponent != null) {
			components.set(componentTypeIndex, null);
			componentsArray.remove(removedComponent);
			componentBits.clear(componentTypeIndex);
			return removedComponent;
		}
		return null;
	}
	
}
