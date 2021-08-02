package org.strobe.gfx.materials.shaders;

import org.joml.Matrix4f;
import org.joml.Vector3f;
import org.strobe.gfx.Graphics;
import org.strobe.gfx.Pool;
import org.strobe.gfx.opengl.bindables.mapper.Uniform;
import org.strobe.gfx.opengl.bindables.shader.ShaderPool;

@Pool(ShaderPool.class)
public final class LambertianColorShader extends MaterialShader{

    private Uniform<Matrix4f> modelMatrix;
    private Uniform<Vector3f> diffuseColor;

    public LambertianColorShader(Graphics gfx) {
        super(gfx, "shaders/LambertianColorShader/");
        modelMatrix = getUniform(Matrix4f.class, "modelMatrix");
        diffuseColor = getUniform(Vector3f.class, "material.diffuseColor");
    }

    @Override
    public void uniformModelMatrix(Graphics gfx, Matrix4f modelMatrix) {
        this.modelMatrix.set(gfx, modelMatrix);
    }

    public void uniformDiffuseColor(Graphics gfx, Vector3f diffuseColor){
        this.diffuseColor.set(gfx, diffuseColor);
    }
}
