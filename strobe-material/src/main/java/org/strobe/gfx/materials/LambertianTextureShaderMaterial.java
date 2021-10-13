package org.strobe.gfx.materials;

import org.joml.Vector3f;
import org.strobe.gfx.Graphics;
import org.strobe.gfx.materials.shaders.LambertianTextureShader;
import org.strobe.gfx.opengl.bindables.shader.ShaderUniformSet;
import org.strobe.gfx.opengl.bindables.texture.Texture2D;

public final class LambertianTextureShaderMaterial extends ShaderMaterial {

    private LambertianTextureShader shader;
    private ShaderUniformSet uniformSet;

    private final Vector3f diffuseColor = new Vector3f();
    private Texture2D diffuseTexture = null;

    public LambertianTextureShaderMaterial(Graphics gfx) {
        super(gfx);
    }

    @Override
    protected MaterialPass[] createPasses(Graphics gfx) {

        shader = new LambertianTextureShader(gfx);
        uniformSet = shader.createShaderUniformSet(gfx);

        return new MaterialPass[]{
                new MaterialPass(shader, MaterialPass.FORWARD_PASS, uniformSet),
                new MaterialPass(null, MaterialPass.SHADOW_MAP_PASS)};
    }

    public void setDiffuseColor(Vector3f diffuseColor){
        diffuseColor.set(diffuseColor);
        uniformSet.set("material.diffuseColor", diffuseColor);
    }

    public Vector3f getDiffuseColor(){
        return diffuseColor;
    }

    public void setDiffuseTexture(Texture2D diffuseTexture){
        this.diffuseTexture = diffuseTexture;
        uniformSet.set("diffuseTexture", diffuseTexture);
    }
}
