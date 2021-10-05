package org.strobe.gfx.opengl.bindables.ubo;

import org.joml.Vector4f;
import org.strobe.gfx.Graphics;
import org.strobe.gfx.opengl.bindables.mapper.Uniform;

public final class UboUniformVector4f implements Uniform<Vector4f> {

    private final Ubo ubo;
    private final int offset;

    private float[] value = new float[4];

    public UboUniformVector4f(Ubo ubo, int offset) {
        this.ubo = ubo;
        this.offset = offset;
    }

    @Override
    public void set(Graphics gfx, Vector4f value) {
        this.value[0] = value.x;
        this.value[1] = value.y;
        this.value[2] = value.z;
        this.value[3] = value.w;
        ubo.bufferSubData(gfx, offset, this.value);
    }

    /**
     * this unbinds the ubo after retrieving the data
     * @param gfx the graphics
     * @return
     */
    @Override
    public Vector4f get(Graphics gfx) {
        float[] value = ubo.getBufferSubDataAsFloatArray(gfx, offset,4);
        return new Vector4f(value);
    }
}
