package org.strobe.gfx.rendergraph.core;

import org.strobe.gfx.Graphics;
import org.strobe.gfx.Renderer;

import java.util.*;

public abstract class RenderGraphRenderer extends RenderPass implements Renderer<GraphDrawable> {

    private record Linkage<T>(Resource<T> source, Resource<T> sink) {
    }

    private final Set<RenderPass> passes = new HashSet<>();
    private final List<RenderPass> queue = new ArrayList<>();
    private boolean requestUpdate = false;
    private boolean requestResourceUpdate = false;
    private final LinkedList<Runnable> graphOps = new LinkedList<>();

    private final ArrayList<Linkage> linkages = new ArrayList<>();

    public RenderGraphRenderer() {
        passes.add(this);
    }


    public <T> Resource<T> registerResource(Class<T> resourceClass, String name, T value) {
        Resource<T> prev = getResource(resourceClass, name);
        if (prev != null) requestResourceUpdate = true;
        Resource<T> resource = super.registerResource(resourceClass, name);
        resource.makeSource(value);
        return resource;
    }

    public <T> Resource<T> registerResource(Class<T> resourceClass, String name) {
        throw new UnsupportedOperationException();
    }

    public void unregisterResource(String name) {
        super.unregisterResource(name);
    }

    public void addPass(RenderPass pass) {
        requestUpdate = true;
        graphOps.add(() -> {
            if (!passes.add(pass)) throw new IllegalStateException("pass is already a part of this RenderGraph");
        });

    }

    public void removePass(RenderPass pass) {
        requestUpdate = true;
        graphOps.add(() -> {
            boolean removed = passes.remove(pass);
            if (!removed) throw new IllegalStateException("can't remove pass");
        });
    }

    public void removeAllPasses() {
        requestUpdate = true;
        graphOps.add(() -> passes.clear());
    }

    public <T> void addLinkage(Resource<T> source, Resource<T> sink) {
        requestUpdate = true;
        graphOps.add(() -> {
            if (!passes.contains(source.getPass()) || !passes.contains(sink.getPass()))
                throw new IllegalStateException("can't add linkage between passes that are not " +
                        "of the RenderGraph");
            linkages.add(new Linkage(source, sink));
        });
    }

    public <T> void addLinkedResourceRoute(Resource<T>... route) {
        if (route.length < 2) throw new IllegalArgumentException();
        for (int i = 1; i < route.length; i++) {
            addLinkage(route[i - 1], route[i]);
        }
    }

    public <T> void removeLinkage(Resource<T> source, Resource<T> sink) {
        requestUpdate = true;
        graphOps.add(() -> {
            boolean removed = linkages.remove(new Linkage(source, sink));
            if (!removed) throw new IllegalStateException("can't remove linkage");
        });
    }

    public void removeAllLinkages() {
        requestUpdate = true;
        graphOps.add(() -> linkages.clear());
    }

    @Override
    public void draw(Graphics gfx, GraphDrawable drawable) {
        if(drawable.getRenderable() == null || drawable.getTransform() == null || drawable.techniques().isEmpty())return;
        for(Technique technique : drawable.techniques()){
            if(!technique.isEnabled())continue;
            for(Step step : technique.steps()){
                step.getRenderQueue().submit(drawable, step);
            }
        }

    }

    public void beforeRender(Graphics gfx) {
    }

    @Override
    public final void render(Graphics gfx) {
        if (requestUpdate) {
            reset(gfx);
            processGraphOps();
            connectResources();
            buildQueue();
            complete(gfx);
            requestUpdate = false;
            requestResourceUpdate = false;
        } else if (requestResourceUpdate) {
            reset(gfx);
            connectResources();
            complete(gfx);
        }
        beforeRender(gfx);
        for (RenderPass pass : queue) {
            pass.render(gfx);
        }
        afterRender(gfx);
    }

    public void afterRender(Graphics gfx) {
    }

    @Override
    public void dispose(Graphics gfx) {
        for (RenderPass pass : passes) {
            if (pass != this)
                pass.dispose(gfx);
        }
    }

    protected void reset(Graphics gfx) {
        for (RenderPass pass : queue) {
            pass.reset(gfx);
            for (Resource resource : pass.resources()) {
                resource.reset();
            }
        }
    }

    private void processGraphOps() {
        while (!graphOps.isEmpty()) graphOps.pop().run();
    }

    private void connectResources() {
        //connect the resources via next and prev
        for (Linkage linkage : linkages) {
            linkage.sink.setPrev(linkage.source);
            linkage.source.setNext(linkage.sink);
        }
        //iterate over all global resources (resources of this graph)
        for (Resource resource : resources()) {
            Object value = resource.get();
            Resource curr = resource.getNext();
            while (curr != null) {
                curr.set(value);
                curr = curr.getNext();
            }
        }
    }

    private void buildQueue() {
        queue.clear();
        Set<RenderPass> visited = new HashSet<>();
        visited.add(this);
        recBuildGraph(this, visited);
    }

    private void recBuildGraph(RenderPass current, Set<RenderPass> visited) {
        visited.add(current);
        for (Resource resource : current.resources()) {
            if (resource.getPrev() == null) continue;
            if (!visited.contains(resource.getPrev().getPass()))
                recBuildGraph(resource.getPrev().getPass(), visited);
        }
        if (current != this) queue.add(current);
        for (Resource resource : current.resources()) {
            if (resource.getNext() == null) continue;
            if (!visited.contains(resource.getNext().getPass()))
                recBuildGraph(resource.getNext().getPass(), visited);
        }
    }

    @Override
    protected void complete(Graphics gfx) {
        for (RenderPass pass : queue) {
            pass.complete(gfx);
        }
    }
}
