package org.strobe.ecs.context.renderer.materials;

import org.strobe.ecs.context.renderer.EntityRenderer;
import org.strobe.gfx.Graphics;
import org.strobe.gfx.materials.GreenShaderMaterial;
import org.strobe.gfx.materials.RedShaderMaterial;
import org.strobe.gfx.materials.ShaderMaterial;

public final class TestMaterial extends Material {

    private GreenShaderMaterial greenShaderMaterial;
    private RedShaderMaterial redShaderMaterial;

    @Override
    protected ShaderMaterial createShaderMaterials(Graphics gfx, EntityRenderer renderer) {
        greenShaderMaterial = new GreenShaderMaterial(gfx);
        redShaderMaterial = new RedShaderMaterial(gfx);
        return greenShaderMaterial;
    }

    public void setGreen(){
        switchShaderMaterial(()->greenShaderMaterial);
    }

    public void setRed(){
        switchShaderMaterial(()->redShaderMaterial);
    }


}
