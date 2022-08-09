package org.marshal.event;

import org.marshal.event.intern.Event;
import org.marshal.event.intern.EventListener;

/**
 * An Listener that only listens to FamilyEvents
 * @version 3.0
 * @author Karl
 */
public abstract class FamilyListener extends EventListener<FamilyEvent> {

    protected FamilyListener() {
        super(Event.FAMILY_EVENT);
    }

    public abstract void dispatched(FamilyEvent familyEvent);
}
