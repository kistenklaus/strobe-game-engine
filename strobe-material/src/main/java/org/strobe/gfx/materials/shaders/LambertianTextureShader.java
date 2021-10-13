package org.strobe.gfx.materials.shaders;

import org.joml.Matrix4f;
import org.joml.Vector3f;
import org.strobe.gfx.Graphics;
import org.strobe.gfx.Pool;
import org.strobe.gfx.opengl.bindables.mapper.Uniform;
import org.strobe.gfx.opengl.bindables.shader.ShaderPool;
import org.strobe.gfx.opengl.bindables.texture.Texture2D;

@Pool(ShaderPool.class)
public final class LambertianTextureShader extends MaterialShader{

    private Uniform<Matrix4f> modelMatrix;
    private Uniform<Vector3f> diffuseColor;
    private Uniform<Texture2D> diffuseTexture;
    private Uniform<Texture2D> dirShadowMap;

    public LambertianTextureShader(Graphics gfx) {
        super(gfx, "shaders/LambertianTextureShader");
        modelMatrix = getUniform(Matrix4f.class, "modelMatrix");
        diffuseColor = getUniform(Vector3f.class, "material.diffuseColor");
        diffuseTexture = getUniform(Texture2D.class, "diffuseTexture");
        dirShadowMap = getUniform(Texture2D.class, "dirShadowMap");
    }

    @Override
    public void uniformModelMatrix(Graphics gfx, Matrix4f modelMatrix) {
        this.modelMatrix.set(gfx, modelMatrix);
    }

    public void uniformDiffuseColor(Graphics gfx, Vector3f diffuseColor){
        this.diffuseColor.set(gfx, diffuseColor);
    }

    public void uniformDiffuseTexture(Graphics gfx, Texture2D diffuseTexture){
        this.diffuseTexture.set(gfx, diffuseTexture);
    }

    @Override
    public void uniformShadowMap(Graphics gfx, Texture2D shadowMap) {
        dirShadowMap.set(gfx, shadowMap);
    }
}
