package org.strobe.gfx.lights;

import org.joml.Matrix4f;
import org.joml.Vector4f;
import org.strobe.gfx.Graphics;
import org.strobe.gfx.Pool;
import org.strobe.gfx.opengl.bindables.mapper.Uniform;
import org.strobe.gfx.opengl.bindables.ubo.Ubo;
import org.strobe.gfx.opengl.bindables.ubo.UboPool;

@Pool(UboPool.class)
public final class ShadowUbo extends Ubo {

    private final Uniform<Matrix4f>[] dirLightLightSpace;
    private final Uniform<Vector4f>[] dirLightShadowDim;
    private final Uniform<Integer>[] dirLightIndices;
    private final Uniform<Integer> dirLightCastingCount;

    public ShadowUbo(Graphics gfx) {
        super(gfx, "shadows", 2, true,
                "mat4[" + LightConstants.MAX_CASTING_LIGHTS + "] directionalLightLightSpace",
                "vec4[" + LightConstants.MAX_CASTING_LIGHTS + "] directionalLightShadowDim",
                "int[" + LightConstants.DIRECTIONAL_LIGHT_COUNT + "] directionalLightIndex",
                "int directionalLightCastingCount");
        dirLightLightSpace = getUniformArray(Matrix4f.class, "directionalLightLightSpace");
        dirLightShadowDim = getUniformArray(Vector4f.class, "directionalLightShadowDim");
        dirLightIndices = getUniformArray(int.class, "directionalLightIndex");
        dirLightCastingCount = getUniform(int.class, "directionalLightCastingCount");
    }

    public void uniformDirLightLightSpaces(Graphics gfx, Matrix4f[] lightSpaces) {
        for (int i = 0; i < lightSpaces.length; i++) {
            dirLightLightSpace[i].set(gfx, lightSpaces[i]);
        }
    }

    public void uniformDirLightShadowDims(Graphics gfx, Vector4f[] shadowDims) {
        for (int i = 0; i < shadowDims.length; i++) {
            dirLightShadowDim[i].set(gfx, shadowDims[i]);
        }
    }

    public void uniformDirLightIndices(Graphics gfx, int[] lightIndices) {
        if (lightIndices.length != LightConstants.DIRECTIONAL_LIGHT_COUNT) throw new IllegalArgumentException();
        for (int i = 0; i < dirLightIndices.length; i++) {
            dirLightIndices[i].set(gfx, lightIndices[i]);
        }
    }

    public void uniformDirLightCastingCount(Graphics gfx, int dirCastingCount) {
        dirLightCastingCount.set(gfx, dirCastingCount);
    }

    public Matrix4f[] getDirLightSpaces(Graphics gfx) {
        Matrix4f[] lightSpaces = new Matrix4f[dirLightLightSpace.length];
        for (int i = 0; i < lightSpaces.length; i++) {
            lightSpaces[i] = dirLightLightSpace[i].get(gfx);
        }
        return lightSpaces;
    }
}
