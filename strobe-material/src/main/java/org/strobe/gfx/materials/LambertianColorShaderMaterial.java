package org.strobe.gfx.materials;

import org.joml.Vector3f;
import org.strobe.gfx.Bindable;
import org.strobe.gfx.Graphics;
import org.strobe.gfx.materials.shaders.LambertianColorShader;
import org.strobe.gfx.materials.shaders.ShadowMapShader;
import org.strobe.gfx.opengl.bindables.shader.ShaderUniformSet;

public final class LambertianColorShaderMaterial extends ShaderMaterial {

    private LambertianColorShader lambertianShader;
    private ShadowMapShader shadowMapShader;
    private ShaderUniformSet uniformSet;

    public LambertianColorShaderMaterial(Graphics gfx) {
        super(gfx);
    }

    @Override
    protected MaterialPass[] createPasses(Graphics gfx) {
        lambertianShader = new LambertianColorShader(gfx);
        uniformSet = lambertianShader.createShaderUniformSet(gfx);

        shadowMapShader = new ShadowMapShader(gfx);

        return new MaterialPass[]{
                new MaterialPass(lambertianShader, MaterialPass.FORWARD_PASS, uniformSet),
                        new MaterialPass(null, MaterialPass.SHADOW_MAP_PASS)};
    }

    public void setDiffuseColor(Vector3f diffuseColor) {
        uniformSet.set("material.diffuseColor", diffuseColor);
    }
}
