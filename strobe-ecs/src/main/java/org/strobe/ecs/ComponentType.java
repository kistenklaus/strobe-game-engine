package org.strobe.ecs;


import org.strobe.utils.UnboundedArray;

import java.util.HashMap;
import java.util.Objects;

final class ComponentType {

    private static final UnboundedArray<ComponentType> componentTypes = new UnboundedArray<>(8);
    private static final HashMap<Class<? extends Component>, Integer> componentIndexMap = new HashMap<>();

    protected static void resetComponentTypes() {
        componentTypes.clear();
        componentIndexMap.clear();
    }

    protected static ComponentType getFor(Class<? extends Component> componentClass) {
        Integer componentIndex = componentIndexMap.get(componentClass);
        if (componentIndex != null) return componentTypes.get(componentIndex);

        Class parentClass = componentClass.getSuperclass();
        if (Component.class.isAssignableFrom(parentClass)) {
            ComponentType parentType = getFor(parentClass);
            componentIndexMap.put(componentClass, parentType.getIndex());
            return parentType;
        } else {
            componentIndex = componentTypes.getNextFreeIndex();
            ComponentType componentType = new ComponentType(componentIndex, componentClass);
            componentTypes.set(componentIndex, componentType);
            componentIndexMap.put(componentClass, componentIndex);
            return componentType;
        }
    }

    private final int componentIndex;
    private final Class<? extends Component> componentClass;

    private ComponentType(int componentIndex, Class<? extends Component> componentClass) {
        this.componentIndex = componentIndex;
        this.componentClass = componentClass;
    }

    public int getIndex() {
        return componentIndex;
    }

    public Class<? extends Component> getComponentClass(){
        return componentClass;
    }

    @Override
    public String toString() {
        return "ComponentType{" +
                "componentIndex=" + componentIndex +
                '}';
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        ComponentType that = (ComponentType) o;
        return componentIndex == that.componentIndex;
    }

    @Override
    public int hashCode() {
        return Objects.hash(componentIndex);
    }
}
