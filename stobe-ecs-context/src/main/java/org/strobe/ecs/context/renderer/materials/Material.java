package org.strobe.ecs.context.renderer.materials;

import org.strobe.ecs.Component;
import org.strobe.ecs.context.renderer.EntityRenderer;
import org.strobe.gfx.Graphics;
import org.strobe.gfx.materials.MaterialPass;
import org.strobe.gfx.materials.ShaderMaterial;
import org.strobe.gfx.rendergraph.core.RenderQueue;
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

    protected final void switchShaderMaterial(final ShaderMaterial shaderMaterial){
        materialOps.add((gfx,r)->{
            ShaderMaterial mat = shaderMaterial;
            if(mat == null)throw new IllegalStateException();
            if(mat != current){
                technique.setSteps(createSteps(gfx, r,  shaderMaterial));
                current = mat;
            }
        });
    }

    private Step[] createSteps(Graphics gfx, EntityRenderer renderer, ShaderMaterial material){
        Step[] steps = new Step[material.passCount()];
        for(int i=0;i<steps.length;i++){
            //select queue based on flags (could be out sourced to the EntityRenderer)
            MaterialPass pass = material.passes()[i];
            RenderQueue queue = switch(pass.flags()){
                case MaterialPass.FORWARD_PASS -> renderer.getForwardQueue();
                case MaterialPass.SHADOW_MAP_PASS -> renderer.getShadowQueue();
                default -> throw new IllegalStateException("Unexpected value: " + pass.flags());
            };
            steps[i] = new Step(queue, pass.shader(), pass.bindables());
        }
        return steps;
    }

    public Technique getTechnique() {
        return technique;
    }
}
