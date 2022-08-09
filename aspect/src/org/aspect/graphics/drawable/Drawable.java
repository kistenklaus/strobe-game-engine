package org.aspect.graphics.drawable;

import org.aspect.graphics.renderer.Renderer;
import org.aspect.graphics.renderer.graph.RenderGraph;

import java.util.ArrayList;

public class Drawable {

    private final Mesh mesh;
    private final Transform transform;

    private final ArrayList<Technique> techniques = new ArrayList<>();

    public Drawable(Mesh mesh, Transform modelMatrix) {
        this.mesh = mesh;
        this.transform = modelMatrix;
    }

    public void addTechnique(Technique technique) {
        this.techniques.add(technique);
    }

    public void submit(Renderer renderer, RenderGraph graph) {
        for(Technique technique : techniques){
            technique.submit(renderer, graph, this);
        }
    }

    public Mesh getMesh(){
        return mesh;
    }

    public Transform getTransform(){
        return transform;
    }

    public final ArrayList<Technique> getTechniques(){
        return techniques;
    }
}
