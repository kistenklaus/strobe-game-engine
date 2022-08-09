package org.aspect.graphics.renderer.drawable;

import org.aspect.graphics.bindable.VertexArray;
import org.aspect.graphics.renderer.graph.RenderGraph;

import java.util.ArrayList;

public class Drawable {

    private final VertexArray vao;

    private final ArrayList<Technique> techniques = new ArrayList<>();

    public Drawable(VertexArray vao) {
        this.vao = vao;
    }

    public void addTechnique(Technique technique){
        this.techniques.add(technique);
    }

    public void submit(RenderGraph frame){
        for(Technique technique : techniques){
            technique.submit(frame, this);
        }
    }

    public VertexArray getVertexArray(){
        return vao;
    }

    public final ArrayList<Technique> getTechniques(){
        return techniques;
    }

}
