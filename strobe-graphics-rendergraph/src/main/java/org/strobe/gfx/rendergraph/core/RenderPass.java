package org.strobe.gfx.rendergraph.core;

import org.strobe.debug.Debuggable;
import org.strobe.gfx.Graphics;

import java.util.Collection;
import java.util.HashMap;

public abstract class RenderPass implements Debuggable {

    private final HashMap<String, Resource> resources = new HashMap<>();
    private final HashMap<String, Class> resourceTypes = new HashMap<>();

    public RenderPass() {
    }

    protected <T> Resource<T> registerResource(Class<T> resourceClass, String name) {
        Resource<T> resource = new Resource<>(this);
        resources.put(name, resource);
        resourceTypes.put(name, resourceClass);
        return resource;
    }

    protected void unregisterResource(String name){
        resources.remove(name);
        resourceTypes.remove(name);
    }

    public <T> Resource<T> getResource(Class<T> resourceClass, String name) {
        Class rtype = resourceTypes.get(name);
        if (rtype == null) return null;
        if (!rtype.equals(resourceClass)) throw new IllegalArgumentException(name + " is of type " + rtype);
        return (Resource<T>) resources.get(name);
    }

    protected abstract void complete(Graphics gfx);

    protected abstract void reset(Graphics gfx);

    protected abstract void render(Graphics gfx);

    protected abstract void dispose(Graphics gfx);

    protected Collection<Resource> resources(){
        return resources.values();
    }

    public String toString(){
        String str = super.toString();
        return str.substring(str.lastIndexOf(".")+1);
    }
}
