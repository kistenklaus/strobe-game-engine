package org.strobe.gfx.opengl.bindables.vao;

import org.strobe.gfx.UnconditionalUnbinding;
import org.strobe.gfx.opengl.bindables.buffer.DataBufferPool;

public class VboPool extends DataBufferPool<Vbo> {

    public VboPool() {
        super(new UnconditionalUnbinding());
    }
}
