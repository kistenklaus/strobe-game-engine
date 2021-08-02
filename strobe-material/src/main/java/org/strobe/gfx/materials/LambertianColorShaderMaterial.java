package org.strobe.gfx.materials;

import org.joml.Vector3f;
import org.strobe.gfx.Bindable;
import org.strobe.gfx.Graphics;
import org.strobe.gfx.materials.shaders.LambertianColorShader;
import org.strobe.gfx.opengl.bindables.shader.ShaderUniformSet;

public final class LambertianColorShaderMaterial extends ShaderMaterial{

    private LambertianColorShader shader;
    private ShaderUniformSet uniformSet;

    public LambertianColorShaderMaterial(Graphics gfx) {
        super(gfx);
    }

    @Override
    protected MaterialPass[] createPasses(Graphics gfx) {
        shader = new LambertianColorShader(gfx);
        uniformSet = shader.createShaderUniformSet(gfx);
        return new MaterialPass[]{new MaterialPass(shader, 0,
                new Bindable[]{uniformSet})};
    }

    public void setDiffuseColor(Vector3f diffuseColor){
        uniformSet.set("material.diffuseColor", diffuseColor);
    }
}
