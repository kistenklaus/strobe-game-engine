package org.strobe.gfx.opengl.bindables.ubo;

import org.joml.Vector3f;
import org.strobe.gfx.Graphics;
import org.strobe.gfx.opengl.bindables.mapper.Uniform;

public class UboUniformVector3f implements Uniform<Vector3f> {

    private final Ubo ubo;
    private final int offset;
    private final float[] vec3_array = new float[3];

    protected UboUniformVector3f(Ubo ubo, int offset) {
        this.ubo = ubo;
        this.offset = offset;
    }

    @Override
    public void set(Graphics gfx, Vector3f vec3) {
        vec3_array[0] = vec3.x;
        vec3_array[1] = vec3.y;
        vec3_array[2] = vec3.z;
        ubo.bufferSubData(gfx, offset, vec3_array);
    }

    @Override
    public Vector3f get(Graphics gfx) {
        float[] value = ubo.getBufferSubDataAsFloatArray(gfx, offset, 3);
        return new Vector3f(value);
    }
}
