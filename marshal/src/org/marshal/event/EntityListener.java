package org.marshal.event;

import org.marshal.event.intern.Event;
import org.marshal.event.intern.EventListener;

/**
 * An EventListener that listens only to EntityEvents
 */
public abstract class EntityListener extends EventListener<EntityEvent> {


    protected EntityListener() {
        super(Event.ENTITY_EVENT);
    }

    public abstract void dispatched(EntityEvent entityEvent);

}
