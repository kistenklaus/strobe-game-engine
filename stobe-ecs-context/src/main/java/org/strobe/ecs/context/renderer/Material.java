package org.strobe.ecs.context.renderer;

import org.strobe.ecs.Component;
import org.strobe.gfx.Graphics;
import org.strobe.gfx.materials.ShaderMaterial;
import org.strobe.gfx.rendergraph.core.Step;
import org.strobe.gfx.rendergraph.core.Technique;

import java.util.LinkedList;
import java.util.function.BiConsumer;
import java.util.function.Supplier;

public abstract class Material implements Component {

    private final Technique technique = new Technique();
    private ShaderMaterial current = null;

    protected final LinkedList<BiConsumer<Graphics, EntityRenderer>> materialOps = new LinkedList<>();

    public Material(){
        materialOps.add((gfx,r)->{
            current = createShaderMaterials(gfx, r);
            technique.setSteps(createSteps(gfx, r, current));
        });
    }

    protected abstract ShaderMaterial createShaderMaterials(Graphics gfx, EntityRenderer renderer);

    void enqueueMaterialOps(EntityRenderer renderer){
        renderer.enqueueRenderOps(materialOps);
        materialOps.clear();
    }

    protected final void switchShaderMaterial(Supplier<ShaderMaterial> material){
        materialOps.add((gfx,r)->{
            ShaderMaterial mat = material.get();
            if(mat == null)throw new IllegalStateException();
            if(mat != current){
                technique.setSteps(createSteps(gfx, r,  material.get()));
                current = mat;
            }
        });
    }

    private Step[] createSteps(Graphics gfx, EntityRenderer renderer, ShaderMaterial material){
        System.out.println("creating steps for " + material);
        Step[] steps = new Step[material.passCount()];
        for(int i=0;i<steps.length;i++){
            //select queue based on flags (could be out sourced to the EntityRenderer)
            steps[i] = new Step(renderer.getForwardQueue(), material.passes()[i].getShader(),
                    material.passes()[i].getBindables());
        }
        return steps;
    }

    public Technique getTechnique() {
        return technique;
    }
}
