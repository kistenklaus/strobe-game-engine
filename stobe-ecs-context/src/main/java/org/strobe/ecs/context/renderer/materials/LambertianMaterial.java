package org.strobe.ecs.context.renderer.materials;

import org.joml.Vector3f;
import org.strobe.ecs.context.renderer.EntityRenderer;
import org.strobe.gfx.Graphics;
import org.strobe.gfx.materials.LambertianColorShaderMaterial;
import org.strobe.gfx.materials.ShaderMaterial;

public final class LambertianMaterial extends Material{

    private LambertianColorShaderMaterial shaderMaterial;

    @Override
    protected ShaderMaterial createShaderMaterials(Graphics gfx, EntityRenderer renderer) {
        shaderMaterial = new LambertianColorShaderMaterial(gfx);
        return shaderMaterial;
    }

    public void setDiffuseColor(Vector3f diffuseColor){
        shaderMaterial.setDiffuseColor(diffuseColor);
    }
}
