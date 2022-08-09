package org.marshal.event;

import org.marshal.Entity;
import org.marshal.event.intern.Event;

/**
 * An Event dispatched on Entity changes
 * @version 3.0
 * @author Karl
 */
public class EntityEvent extends Event {


    public static final int ENTITY_ADDED = 0b1;
    public static final int ENTITY_REMOVED = 0b10;

    private final Entity entity;

    public EntityEvent(Entity entity, int mods) {
        super(Event.ENTITY_EVENT, mods);
        this.entity = entity;
    }

    public Entity getEntity(){
        return entity;
    }
}
