package strb.ecs;

import java.util.HashMap;
import strb.util.Bits;

class ComponentType {
	
	private static HashMap<Class<? extends Component>,ComponentType> assignedComponentTypes = new HashMap<>();
	private static int typeIndex = 0;
	
	private final int index;
	
	private ComponentType() {
		index = typeIndex++;
	}
	
	public int getIndex() {
		return index;
	}
	
	static ComponentType getFor(Class<? extends Component> component_class) {
		ComponentType type = assignedComponentTypes.get(component_class);
		if(type==null) {
			type = new ComponentType();
			assignedComponentTypes.put(component_class, type);
		}
		return type;
	}
	static int getIndexFor(Class<? extends Component> component_class) {
		return getFor(component_class).getIndex();
	}
	@SafeVarargs
	static Bits getBitsFor(Class<? extends Component>... component_classes) {
		Bits bits = new Bits();
		
		int types_amount = component_classes.length;
		for(int i=0;i<types_amount; i++) {
			bits.set(ComponentType.getIndexFor(component_classes[i]));
		}
		return bits;
	}
	@Override
	public int hashCode() {
		return index;
	}
	@Override
	public boolean equals(Object obj) {
		if (this == obj) return true;
		if (obj == null) return false;
		if (getClass() != obj.getClass()) return false;
		ComponentType other = (ComponentType)obj;
		return index == other.index;
	}
}
