package strb.ecs;


public class ComponentMapper<T extends Component> {
	
	private final ComponentType componentType;
	
	private ComponentMapper(ComponentType componentType) {
		this.componentType = componentType;
	}
	
	public static <E extends Component> ComponentMapper<E> getFor(Class<E> component_class){
		return new ComponentMapper<E>(ComponentType.getFor(component_class));
	}
	
	public T get(Entity entity) {
		return entity.getComponent(componentType);
	}
	
	public boolean has(Entity entity) {
		return entity.hasComponent(componentType);
	}
	
}
