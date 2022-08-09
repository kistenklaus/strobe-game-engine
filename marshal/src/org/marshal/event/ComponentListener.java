package org.marshal.event;

import org.marshal.event.intern.Event;
import org.marshal.event.intern.EventListener;


/**
 * An EventListener that listens to all ComponentEvents
 * @version 3.0
 * @author Karl
 */
public abstract class ComponentListener extends EventListener<ComponentEvent> {

    protected ComponentListener() {
        super(Event.COMPONENT_EVENT);
    }

    public abstract void dispatched(ComponentEvent componentEvent);
}
