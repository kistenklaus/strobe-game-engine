package org.marshal.event.intern;


/**
 * A Listener that only listens to Events
 * @param <T> the event Type
 * @version 3.0
 * @author Karl
 */
public abstract class EventListener<T extends Event> implements Listener<T> {

    private final int eventTypeIndex;

    protected EventListener(int eventTypeIndex){
        this.eventTypeIndex = eventTypeIndex;
    }

    public abstract void dispatched(T event);

    public boolean filter(T event){
        return true;
    }

    public int getEventTypeIndex(){
        return this.eventTypeIndex;
    }
}
