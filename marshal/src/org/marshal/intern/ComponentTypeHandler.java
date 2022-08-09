package org.marshal.intern;

import org.marshal.Component;

import java.util.HashMap;

/**
 * handles all component types
 * @version 3.0
 * @author Karl
 */
public class ComponentTypeHandler {

    private final HashMap<Class<? extends Component>, ComponentType>
            componentTypes = new HashMap<>();

    private int componentIndexAcc = 0;

    public ComponentTypeHandler(){
    }

    public synchronized ComponentType getFor(Class<? extends Component> componentClass) {
        ComponentType type = componentTypes.get(componentClass);
        if(type!=null)return type;
        type = createComponentType(componentClass);
        componentTypes.put(componentClass, type);
        return type;
    }

    private ComponentType createComponentType(
            Class<? extends Component> componentClass) {

        int componentIndex = componentIndexAcc++;

        Class<?> assignableFrom = componentClass;
        while ((assignableFrom =
                assignableFrom.getSuperclass()) != Component.class) {
            ComponentType superType =
                    getFor((Class<? extends Component>) assignableFrom);
            superType.addChildType(componentIndex);
        }

        return new ComponentType(componentIndex);
    }

    public int getCurrentMaxComponentIndex(){
        return componentIndexAcc;
    }

}
