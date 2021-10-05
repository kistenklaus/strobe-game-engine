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

    private final Uniform<Matrix4f>[] dirLightLightSpaces;
    private final Matrix4f[] dirLightLightSpacesLocal = new Matrix4f[LightConstants.MAX_DIR_CASTING_LIGHTS];
    private final Uniform<Vector4f>[] dirLightShadowDims;
    private final Vector4f[] dirLightShadowDimsLocal = new Vector4f[LightConstants.MAX_DIR_CASTING_LIGHTS];
    private final Uniform<Integer>[] dirLightIndices;
    private final Uniform<Integer> dirLightCastingCount;
    private int dirLightCastingCountLocal = 0;

    public ShadowUbo(Graphics gfx) {
        super(gfx, "shadows", 2, true,
                "mat4[" + LightConstants.MAX_DIR_CASTING_LIGHTS + "] directionalLightLightSpace",
                "vec4[" + LightConstants.MAX_DIR_CASTING_LIGHTS + "] directionalLightShadowDim",
                "int[" + LightConstants.DIRECTIONAL_LIGHT_COUNT + "] directionalLightIndex",
                "int directionalLightCastingCount");
        dirLightLightSpaces = getUniformArray(Matrix4f.class, "directionalLightLightSpace");
        dirLightShadowDims = getUniformArray(Vector4f.class, "directionalLightShadowDim");
        dirLightIndices = getUniformArray(int.class, "directionalLightIndex");
        dirLightCastingCount = getUniform(int.class, "directionalLightCastingCount");
    }

    public void uniformDirLightLightSpaces(Graphics gfx, Matrix4f[] lightSpaces) {
        for (int i = 0; i < lightSpaces.length; i++) {
            dirLightLightSpaces[i].set(gfx, lightSpaces[i]);
            dirLightLightSpacesLocal[i] = lightSpaces[i];
        }

        //TODO probably not needed because the data is not used
        Matrix4f identity = new Matrix4f().identity();
        for(int i = lightSpaces.length; i<LightConstants.MAX_DIR_CASTING_LIGHTS; i++){
            dirLightLightSpaces[i].set(gfx, identity);
            dirLightLightSpacesLocal[i] = identity;
        }
    }

    public void uniformDirLightShadowDims(Graphics gfx, Vector4f[] shadowDims) {
        for (int i = 0; i < shadowDims.length; i++) {
            dirLightShadowDims[i].set(gfx, shadowDims[i]);
            dirLightShadowDimsLocal[i] = shadowDims[i];
        }
        //TODO probably not needed because the data is not used
        Vector4f def = new Vector4f(0);
        for(int i = shadowDims.length; i<LightConstants.MAX_DIR_CASTING_LIGHTS; i++){
            dirLightShadowDims[i].set(gfx, def);
            dirLightShadowDimsLocal[i] = def;
        }
    }

    public void uniformDirLightIndices(Graphics gfx, int[] lightIndices) {
        if (lightIndices.length != LightConstants.DIRECTIONAL_LIGHT_COUNT) throw new IllegalArgumentException();
        for (int i = 0; i < dirLightIndices.length; i++) {
            dirLightIndices[i].set(gfx, lightIndices[i]);
        }
    }

    public void uniformDirLightCastingCount(Graphics gfx, int dirCastingCount) {
        if(dirCastingCount < 0)throw new IllegalArgumentException();
        dirLightCastingCount.set(gfx, dirCastingCount);
        dirLightCastingCountLocal = dirCastingCount;
    }

    public Matrix4f[] getDirLightSpaces() {
        return dirLightLightSpacesLocal;
    }

    public int getDirLightCastingCount() {
        return dirLightCastingCountLocal;
    }

    public Matrix4f[] nativeFetchDirLightSpaces(Graphics gfx){
        Matrix4f[] out = new Matrix4f[LightConstants.MAX_DIR_CASTING_LIGHTS];
        for(int i=0;i<LightConstants.MAX_DIR_CASTING_LIGHTS;i++){
            out[i] = dirLightLightSpaces[i].get(gfx);
        }
        return out;
    }

    public int[] nativeFetchDirIndices(Graphics gfx){
        int[] indices = new int[LightConstants.DIRECTIONAL_LIGHT_COUNT];
        for(int i=0;i<LightConstants.DIRECTIONAL_LIGHT_COUNT;i++){
            indices[i] = dirLightIndices[i].get(gfx);
        }
        return indices;
    }

    public int nativeFetchDirCastingCount(Graphics gfx) {
        return dirLightCastingCount.get(gfx);
    }
}
