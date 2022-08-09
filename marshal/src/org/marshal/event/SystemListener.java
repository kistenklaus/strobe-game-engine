package org.marshal.event;

import org.marshal.event.intern.Event;
import org.marshal.event.intern.EventListener;

/**
 * A Listener that only listens to SystemEvents
 * @version 3.0
 * @author Karl
 */
public abstract class SystemListener extends EventListener<SystemEvent> {

    protected SystemListener() {
        super(Event.SYSTEM_EVENT);
    }

    public abstract void dispatched(SystemEvent systemEvent);
}
