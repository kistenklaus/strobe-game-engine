package org.marshal.event;

import org.marshal.systems.EntitySystem;
import org.marshal.event.intern.Event;

/**
 * An Event dispatched on System changes
 * @version 3.0
 * @author Karl
 */
public class SystemEvent extends Event {

    public static final int SYSTEM_ADDED = 0b1;
    public static final int SYSTEM_REMOVED = 0b10;

    private final EntitySystem system;

    public SystemEvent(EntitySystem system, int eventModifiers) {
        super(Event.SYSTEM_EVENT, eventModifiers);
        this.system = system;
    }

    public EntitySystem getSystem(){
        return system;
    }
}
