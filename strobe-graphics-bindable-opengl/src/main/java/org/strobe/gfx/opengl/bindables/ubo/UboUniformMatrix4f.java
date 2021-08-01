package org.strobe.gfx.opengl.bindables.ubo;

import org.joml.Matrix4f;
import org.strobe.gfx.Graphics;
import org.strobe.gfx.opengl.bindables.mapper.Uniform;

public class UboUniformMatrix4f implements Uniform<Matrix4f> {

    private final Ubo ubo;
    private final int offset;
    private final float[] mat4_array = new float[16];

    protected UboUniformMatrix4f(Ubo ubo, int offset) {
        this.ubo = ubo;
        this.offset = offset;
    }

    @Override
    public void set(Graphics gfx, Matrix4f value) {
        value.get(mat4_array);
        ubo.bufferSubData(gfx, offset, mat4_array);
    }

    @Override
    public Matrix4f get(Graphics gfx) {
        float[] value = ubo.getBufferSubDataAsFloatArray(gfx, offset, 16);
        return new Matrix4f(value[0], value[1], value[2], value[3], value[4], value[5], value[6], value[7],
                value[8], value[9], value[10], value[11], value[12], value[13], value[14], value[15]);
    }
}
