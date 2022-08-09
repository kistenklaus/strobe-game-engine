package org.marshal;

import org.marshal.event.intern.Event;
import org.marshal.event.intern.EventListener;

import java.util.LinkedList;
import java.util.Queue;


/**
 * handles all events and all event listeners attached to thread of the eventHandler
 * @version 3.0
 * @author Karl
 */
class EventHandler {

    private final Queue<Event> eventQueue = new LinkedList<>();

    private final LinkedList<EventListener<Event>>[] listeners = new LinkedList[Event.EVENT_TYPE_LAST];

    public EventHandler() {
        for (int i = 0; i < Event.EVENT_TYPE_LAST; i++) {
            listeners[i] = new LinkedList<>();
        }
    }

    public void pollEvents() {
        Event event;
        while ((event = eventQueue.poll()) != null) {
            int typeIndex = event.getTypeIndex();
            LinkedList<EventListener<Event>> listeners = this.listeners[typeIndex];
            for (EventListener<Event> listener : listeners) {
                if (listener.filter(event)) {
                    listener.dispatched(event);
                }
            }
        }
    }

    public void queueEvent(Event event) {
        eventQueue.add(event);
    }

    public void addListener(EventListener eventListener) {
        int typeIndex = eventListener.getEventTypeIndex();
        LinkedList<EventListener<Event>> listeners = this.listeners[typeIndex];
        listeners.add(eventListener);
    }
}
