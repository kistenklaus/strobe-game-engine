package org.marshal.event;

import org.marshal.Component;
import org.marshal.Entity;
import org.marshal.event.intern.Event;


/**
 * A Event dispatched on component changes
 * @version 3.0
 * @author Karl
 */
public class ComponentEvent extends Event {

    public static final int COMPONENT_ADDED = 0b1;
    public static final int COMPONENT_REMOVED = 0b10;

    private final Entity entity;
    private final Component component;

    public ComponentEvent(Entity entity, Component component, int mods) {
        super(Event.COMPONENT_EVENT, mods);
        this.entity = entity;
        this.component = component;
    }

    public Entity getEntity() {
        return entity;
    }

    public Component getComponent() {
        return component;
    }
}
