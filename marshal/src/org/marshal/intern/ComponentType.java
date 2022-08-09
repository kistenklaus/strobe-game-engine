package org.marshal.intern;


/**
 * a component type is the marshal way of working with component classes
 * an component type is unique for every component class, and as a unique index
 * assigned to it.
 *
 * @version 3.0
 * @author Karl
 */
public class ComponentType {

    private final int componentIndex;
    private final ComponentMask assignableComponents = new ComponentMask();


    public ComponentType(int componentIndex) {
        this.componentIndex = componentIndex;
        assignableComponents.set(componentIndex, true);
    }

    public int getIndex() {
        return componentIndex;
    }

    public void addChildType(int childIndex){
        assignableComponents.set(childIndex, true);
    }

    public ComponentMask getAssignableComponents() {
        return assignableComponents;
    }
}
