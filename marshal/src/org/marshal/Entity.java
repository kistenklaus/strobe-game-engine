package org.marshal;

import org.marshal.intern.ComponentMask;
import org.marshal.intern.ComponentType;

import java.util.ArrayList;

/**
 * An entity is a container for Components
 * @version 3.0
 * @author Karl
 */
public final class Entity {

    private final ArrayList<Component> components;
    private final ComponentMask componentMask = new ComponentMask();
    private final EntityComponentSystem ecs;

    private int phase = 0;

    private boolean active = false;

    Entity(EntityComponentSystem ecs) {
        this.ecs = ecs;
        //allocates space for components
        final int maxComponentIndex
                = ecs.getComponentTypeHandler().getCurrentMaxComponentIndex();
        this.components = new ArrayList<>(maxComponentIndex);
        for (int i = 0; i < maxComponentIndex; i++) {
            this.components.add(null);
        }
    }


    /**
     * adds an Component to the Entity
     * @param component the component to add
     * @return true when the entity was added
     */
    @SuppressWarnings("unchecked")
    public final boolean add(final Component component) {

        Class<?> componentClass = component.getClass();
        do {
            ComponentType assignableFrom =
                    ecs.registerComponentType(
                            (Class<? extends org.marshal.Component>) componentClass);
            if (componentMask.get(assignableFrom.getIndex())) {
                return false;
            }
        } while ((componentClass = componentClass.getSuperclass()) != Component.class);
        //componentClass = null;


        final int componentIndex = ecs.registerComponentType(component.getClass()).getIndex();
        componentMask.set(componentIndex, true);
        if (components.size() <= componentIndex) {
            //allocates space for components
            components.ensureCapacity(componentIndex * 3 / 2 + 1);
            for (int i = components.size(); i < componentIndex * 3 / 2 + 1; i++)
                components.add(null);
        }
        components.set(componentIndex, component);

        ecs.dispatchComponentAdded(this, component);
        return true;
    }

    /**
     * removes the component from the entity
     * @param component the component to remove
     * @return true when the component was removed
     */
    public boolean remove(Component component) {
        return remove(component.getClass());
    }

    /**
     * removes the component of this component class from the entity
     * @param componentClass the component class to remove
     * @return true when the component was removed
     */
    public boolean remove(Class<? extends Component> componentClass) {
        final int componentIndex = ecs.registerComponentType(componentClass).getIndex();
        if (componentMask.get(componentIndex)) {
            Component removed = components.get(componentIndex);
            components.set(componentIndex, null);
            componentMask.set(componentIndex, false);

            ecs.dispatchComponentRemoved(this, removed);
            return true;
        } else return false;
    }

    /**
     * removes all components from the entity
     */
    public void clear(){
        for (Component component : components) {
            if (component != null) {
                remove(component);
            }
        }
    }

    /**
     * disposes the entity
     * frees the entity instance scheduled for new entities
     */
    public void dispose(){
        ecs.removeEntity(this);
        clear();
        nextPhase();
        ecs.collectEntity(this);
    }

    /**
     * checks if the entity is a part of the ecs
     * @param ecs the ecs to check
     * @return true when the entity was created of the ecs
     */
    boolean isPartOfEcs(EntityComponentSystem ecs){
        return this.ecs.equals(ecs);
    }

    /**
     * increments the phase of the entity
     */
    void nextPhase(){
        phase++;
    }

    /**
     * @return the phase of the entity
     */
    int getPhase(){
        return phase;
    }

    /**
     * @return the component mask of the entity
     */
    public ComponentMask getComponentMask(){
        return componentMask;
    }

    /**
     * activates the entity
     */
    void activate(){
        this.active = true;
    }

    /**
     * deactivates the entity
     */
    void deactivate(){
        this.active = false;
    }

    /**
     * @return true when the entity is active
     */
    public boolean isActive(){
        return this.active;
    }

}
