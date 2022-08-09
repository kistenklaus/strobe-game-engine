package org.marshal.event.intern;


/**
 * abstract idea of a listener
 * @param <T> the type to listen for
 */
public interface Listener<T>{

    void dispatched(T source);
}
