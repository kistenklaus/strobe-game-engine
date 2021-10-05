package org.strobe.gfx.materials.shaders;

import org.joml.Matrix4f;
import org.joml.Vector3f;
import org.strobe.gfx.Graphics;
import org.strobe.gfx.Pool;
import org.strobe.gfx.opengl.bindables.mapper.Uniform;
import org.strobe.gfx.opengl.bindables.shader.ShaderPool;
import org.strobe.gfx.opengl.bindables.texture.Texture2D;

@Pool(ShaderPool.class)
public final class LambertianColorShader extends MaterialShader{

    private Uniform<Matrix4f> modelMatrix;
    private Uniform<Vector3f> diffuseColor;
    private Uniform<Texture2D> dirShadowMap;

    public LambertianColorShader(Graphics gfx) {
        super(gfx, "shaders/LambertianColorShader/");
        modelMatrix = getUniform(Matrix4f.class, "modelMatrix");
        diffuseColor = getUniform(Vector3f.class, "material.diffuseColor");
        dirShadowMap = getUniform(Texture2D.class, "dirShadowMap");
    }

    @Override
    public void uniformModelMatrix(Graphics gfx, Matrix4f modelMatrix) {
        this.modelMatrix.set(gfx, modelMatrix);
    }

    public void uniformDiffuseColor(Graphics gfx, Vector3f diffuseColor){
        this.diffuseColor.set(gfx, diffuseColor);
    }

    @Override
    public void uniformShadowMap(Graphics gfx, Texture2D shadowMap) {
        dirShadowMap.set(gfx, shadowMap);
    }
}
