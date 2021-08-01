package org.strobe.gfx.rendergraph.core;

import org.strobe.debug.Debug;
import org.strobe.gfx.Drawable;

public final class Resource<T> implements Drawable {
    @Debug
    private T value;
    private Resource<T> prev = null;
    private Resource<T> next = null;

    private final RenderPass pass;
    @Debug
    private boolean isSource = false;

    protected Resource(RenderPass pass){
        this.pass = pass;
        isSource = false;
    }

    protected Resource(RenderPass pass, T value){
        this.value = value;
        this.pass = pass;
    }

    protected void makeSource(T value){
        this.value = value;
        isSource = true;
    }

    protected void set(T value){
        this.value = value;
    }

    public T get(){
        return value;
    }

    protected Resource<T> getPrev(){
        return prev;
    }

    protected void setPrev(Resource<T> prev) {
        this.prev = prev;
    }

    protected void setNext(Resource<T> next) {
        this.next = next;
    }

    protected Resource<T> getNext(){
        return next;
    }

    protected RenderPass getPass(){
        return pass;
    }

    protected void reset(){
       if(!isSource) value = null;
       prev = null;
       next = null;
    }

    @Override
    public String toString() {
        return "Resource{" +
                "value=" + value +
                ", pass=" + pass +
                '}';
    }
}

