package org.strobe.gfx.opengl.bindables.ubo;

import org.strobe.gfx.Graphics;
import org.strobe.gfx.opengl.bindables.mapper.Uniform;

public final class UboUniformInteger implements Uniform<Integer> {

    private final Ubo ubo;
    private final int offset;
    private final int[] value;

    public UboUniformInteger(Ubo ubo, int offset){
        this.ubo = ubo;
        this.offset = offset;
        this.value = new int[1];
    }

    @Override
    public void set(Graphics gfx, Integer value) {
        this.value[0] = value;
        ubo.bufferSubData(gfx, offset, this.value);
    }

    /**
     * this unbinds the ubo after retrieving the data
     * @param gfx the graphics
     * @return
     */
    @Override
    public Integer get(Graphics gfx) {
        int[] value = ubo.getBufferSubDataAsIntegerArray(gfx, offset, 1);
        return value[0];
    }
}
