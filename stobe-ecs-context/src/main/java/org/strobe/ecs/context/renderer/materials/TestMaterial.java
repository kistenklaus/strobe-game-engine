package org.strobe.ecs.context.renderer.materials;

import org.strobe.ecs.context.renderer.Material;
import org.strobe.gfx.materials.GreenShaderMaterial;
import org.strobe.gfx.materials.RedShaderMaterial;
import org.strobe.gfx.rendergraph.core.Step;

public final class TestMaterial extends Material {

    private Step greenStep;
    private Step redStep;

    boolean green = true;

    public TestMaterial(){
        materialOps.add((gfx,renderer)->{
            GreenShaderMaterial greenShaderMaterial = new GreenShaderMaterial(gfx);
            greenStep = new Step(renderer.getForwardQueue(), greenShaderMaterial.passes()[0].getShader(), greenShaderMaterial.passes()[0].getBindables());
            technique.setSteps(greenStep);

            RedShaderMaterial redShaderMaterial = new RedShaderMaterial(gfx);
            redStep = new Step(renderer.getForwardQueue(), redShaderMaterial.passes()[0].getShader(), greenShaderMaterial.passes()[0].getBindables());

        });
    }

    public void switchShader(){
        materialOps.add((gfx,r)->{
            if(green){
                technique.setSteps(redStep);
            }else{
                technique.setSteps(greenStep);
            }
            green = !green;
        });
    }

}
