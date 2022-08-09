package marshal;

import java.util.HashMap;

final class MarshalComponentType {
	
	private static int component_index_acc = 0;
	private static HashMap<Class<? extends MarshalComponent>, MarshalComponentType> type_map = new HashMap<>();
	
	@SuppressWarnings("unchecked")
	static <T extends MarshalComponent> MarshalComponentType get(Class<T> component_class) {
		MarshalComponentType component_type = type_map.get(component_class);
		if(component_type!=null)return component_type;
		int component_index = component_index_acc++;
		component_type = new MarshalComponentType(component_index);
		type_map.put(component_class, component_type);
		Class<? super T> super_component_class = component_class.getSuperclass();
		if(!super_component_class.equals(Object.class)) {
			while(!super_component_class.equals(MarshalComponent.class)) {
				MarshalComponentType super_component_type = get((Class<? extends MarshalComponent>)super_component_class);
				super_component_type.inherited_by.put(component_type);
				
				super_component_class = super_component_class.getSuperclass();
			}
		}
		return component_type;
	}
	
	static int getIndex(Class<? extends MarshalComponent> component_class) {
		return get(component_class).index();
	}
	
	private final int component_index;
	MarshalComponentBits inherited_by = new MarshalComponentBits();
	
	private MarshalComponentType(int component_index) {
		this.component_index = component_index;
	}
	
	int index(){
		return component_index;
	}

}
