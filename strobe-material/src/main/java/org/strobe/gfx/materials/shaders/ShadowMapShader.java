package org.strobe.gfx.materials.shaders;

import org.joml.Matrix4f;
import org.strobe.gfx.Graphics;
import org.strobe.gfx.Pool;
import org.strobe.gfx.opengl.bindables.mapper.Uniform;
import org.strobe.gfx.opengl.bindables.shader.ShaderPool;

@Pool(ShaderPool.class)
public final class ShadowMapShader extends MaterialShader{

    private final Uniform<Matrix4f> modelMatrix;
    private final Uniform<Integer> lightIndex;

    public ShadowMapShader(Graphics gfx) {
        super(gfx, "shaders/shadowMap/");
        modelMatrix = getUniform(Matrix4f.class, "modelMatrix");
        lightIndex = getUniform(int.class, "lightIndex");
    }

    @Override
    public void uniformModelMatrix(Graphics gfx, Matrix4f modelMatrix) {
        this.modelMatrix.set(gfx, modelMatrix);
    }

    public void uniformLightIndex(Graphics gfx, int index){
        lightIndex.set(gfx, index);
    }
}
