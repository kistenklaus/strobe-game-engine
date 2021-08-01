package org.strobe.gfx.rendergraph.core;

import org.strobe.gfx.Drawable;
import org.strobe.gfx.Renderable;
import org.strobe.gfx.transform.AbstractTransform;

import java.util.ArrayList;
import java.util.Arrays;

public final class GraphDrawable implements Drawable {

    private Renderable renderable;
    private AbstractTransform transform;
    private ArrayList<Technique> techniques = new ArrayList<>();

    public GraphDrawable(Renderable mesh, AbstractTransform transform, Technique...techniques){
        this.transform = transform;
        this.renderable = mesh;
        this.techniques.addAll(Arrays.asList(techniques));
    }

    public void addTechnique(Technique technique){
        this.techniques.add(technique);
    }

    public void removeTechnique(Technique technique){
        this.techniques.remove(technique);
    }

    public void clearTechniques() {
        this.techniques.clear();
    }

    public Renderable getRenderable() {
        return renderable;
    }

    public AbstractTransform getTransform() {
        return transform;
    }

    public ArrayList<Technique> techniques() {
        return techniques;
    }

    public void setRenderable(Renderable mesh) {
        this.renderable = mesh;
    }

    public void setTransform(AbstractTransform transform) {
        this.transform = transform;
    }

    @Override
    public String toString() {
        return "GraphDrawable{" +
                "mesh=" + renderable +
                ", transform=" + transform +
                ", techniques=" + techniques +
                '}';
    }
}
