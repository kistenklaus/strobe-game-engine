package org.strobe.ecs;

import org.strobe.commons.lang.UnboundedArray;

public final class ComponentMapper<T extends Component> {

    private static final UnboundedArray<ComponentMapper> componentMappers = new UnboundedArray<>(8);

    public static final <T extends Component> ComponentMapper getFor(Class<T> componentClass) {
        ComponentType componentType = ComponentType.getFor(componentClass);
        ComponentMapper mapper = componentMappers.get(componentType.getIndex());
        if (mapper != null) return mapper;
        mapper = new ComponentMapper<T>(componentType);
        componentMappers.set(componentType.getIndex(), mapper);
        return mapper;
    }

    private final ComponentType componentType;

    private ComponentMapper(ComponentType componentType) {
        this.componentType = componentType;
    }

    protected ComponentType getComponentType() {
        return componentType;
    }

    public T get(Entity entity){
        return entity.get(this);
    }

    public boolean has(Entity entity){
        return entity.has(this);
    }

    public boolean remove(Entity entity){
        return entity.removeComponent(this);
    }

    public void add(Entity entity, T component){
        entity.addComponent(this, component);
    }
}
