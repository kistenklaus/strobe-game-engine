package org.strobe.ecs.context.renderer;

import org.strobe.ecs.Component;
import org.strobe.gfx.Graphics;
import org.strobe.gfx.rendergraph.core.Technique;

import java.util.LinkedList;
import java.util.function.BiConsumer;

public abstract class Material implements Component {

    protected final Technique technique = new Technique();

    protected final LinkedList<BiConsumer<Graphics, EntityRenderer>> materialOps = new LinkedList<>();

    void enqueueMaterialOps(EntityRenderer renderer){
        renderer.enqueueRenderOps(materialOps);
        materialOps.clear();
    }

    public Technique getTechnique() {
        return technique;
    }
}
