package org.strobe.gfx.opengl.bindables.vao;

import org.strobe.gfx.UnconditionalUnbinding;
import org.strobe.gfx.opengl.bindables.buffer.DataBufferPool;

public class IboPool extends DataBufferPool<Ibo> {

    public IboPool() {
        super(new UnconditionalUnbinding());
    }
}
