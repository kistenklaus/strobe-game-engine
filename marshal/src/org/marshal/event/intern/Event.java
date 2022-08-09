package org.marshal.event.intern;

/**
 * abstract idea of an Event
 * @version 3.0
 * @author Karl
 */
public abstract class Event {
    public static final int ENTITY_EVENT = 0;
    public static final int COMPONENT_EVENT = ENTITY_EVENT+1;
    public static final int FAMILY_EVENT = COMPONENT_EVENT+1;
    public static final int SYSTEM_EVENT = FAMILY_EVENT+1;
    public static final int EVENT_TYPE_LAST = SYSTEM_EVENT+1;

    private final int eventTypeIndex;
    private final int eventModifiers;

    protected Event(int eventType, int eventModifiers){
        this.eventTypeIndex = eventType;
        this.eventModifiers = eventModifiers;
    }

    public int getTypeIndex(){
        return eventTypeIndex;
    }

    public boolean isMod(int mod){
        return (this.eventModifiers & mod) != 0;
    }
}
