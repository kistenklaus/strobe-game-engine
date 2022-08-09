package marshal;

public final class MarshalEntity {
	
	MarshalSignal<MarshalComponent> component_added = new MarshalSignal<>();
	MarshalSignal<MarshalComponent> component_removed = new MarshalSignal<>();
	
	int cache_index = -1;
	
	MarshalEntity(){
		
	}
	
	MarshalStaticArray<MarshalComponent> components = new MarshalStaticArray<>();
	MarshalComponentBits componentBits = new MarshalComponentBits();
	
	public boolean has(MarshalComponent component) {
		return has(component.getClass());
	}
	
	public boolean has(Class<? extends MarshalComponent> component_class) {
		return has(MarshalComponentType.get(component_class));
	}
	
	boolean has(MarshalComponentType componentType) {
		return componentBits.has(componentType);
	}
	
	public void add(MarshalComponent component) {
		MarshalComponentType component_type = MarshalComponentType.get(component.getClass());
		//int component_index = MarshalComponentType.getIndex(component.getClass());
		MarshalComponent entry = components.get(component_type.index());
		if(entry != null) {
			if(component instanceof MarshalStackableComponent) {
				MarshalStackableComponent stackableComponent = (MarshalStackableComponent) component;
				MarshalStackableComponent stackableEntry = (MarshalStackableComponent) entry;
				MarshalStackableComponent stackableHead = stackableEntry.next;
				while(stackableHead.next != null) {
					stackableHead = stackableHead.next;
				}
				stackableHead.next = stackableComponent;
				component.root = this;
			}else {
				return;
			}
		}else {
			
			componentBits.put(component_type);
			components.set(component_type.index(), component);
			component.root = this;
		}
		component_added.dispatch(component);
	}
	
	public boolean remove(Class<? extends MarshalComponent> component_class) {
		return remove(MarshalComponentType.get(component_class));
	}
	
	public boolean remove(MarshalComponent component) {
		return remove(component.getClass());
	}
	
	boolean remove(MarshalComponentType component_type) {
		int component_index = component_type.index();
		MarshalComponent entry = components.get(component_index);
		if(entry == null)return false;
		if(entry instanceof MarshalStackableComponent) {
			MarshalStackableComponent stackableEntry = (MarshalStackableComponent) entry;
			if(stackableEntry.next == null) {
				componentBits.remove(component_type);
				return components.remove(component_index);
			}
			while(stackableEntry.next.next != null) {
				stackableEntry = stackableEntry.next;
			}
			stackableEntry.next = null;
			component_removed.dispatch(entry);
			return true;
		}else {
			
			componentBits.remove(component_type);
			component_removed.dispatch(entry);
			
			return components.remove(component_index);
		}
	}
	
	public boolean removeAll(Class<? extends MarshalComponent> component_class) {
		return removeAll(MarshalComponentType.get(component_class));
	}
	
	boolean removeAll(MarshalComponentType component_type) {
		int component_index = component_type.index();
		MarshalComponent entry = components.get(component_index);
		if(entry == null)return false;
		componentBits.remove(component_type);
		return components.remove(component_index);
	}
	
	public String toString() {
		StringBuilder str_builder = new StringBuilder("Entity@[");
		for(MarshalComponent component : components) {
			str_builder.append(component.getClass().getSimpleName()+",");
		}
		str_builder.deleteCharAt(str_builder.length()-1);
		str_builder.append("]");
		return str_builder.toString();
	}
	

	
}
