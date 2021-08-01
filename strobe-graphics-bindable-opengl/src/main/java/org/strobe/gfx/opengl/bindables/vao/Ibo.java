package org.strobe.gfx.opengl.bindables.vao;

import org.strobe.gfx.Graphics;
import org.strobe.gfx.Pool;
import org.strobe.gfx.opengl.bindables.buffer.DataBuffer;

import static org.lwjgl.opengl.GL15.*;


@Pool(IboPool.class)
public class Ibo extends DataBuffer<IboPool> {

    public Ibo(Graphics gfx, int[] indices, boolean readOnly) {
        super(gfx, GL_ELEMENT_ARRAY_BUFFER, indices.length * Integer.BYTES, readOnly ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);
        pool.add(this);
        bufferSubData(gfx, 0, indices);
    }

    protected void connectToBoundVao(Graphics gfx) {
        gfx.bind(this);
    }
}
