package org.strobe.gfx.opengl.bindables.framebuffer;


import org.strobe.gfx.Graphics;
import org.strobe.gfx.Pool;

@Pool(FramebufferPool.class)
class FramebufferBackBuffer extends Framebuffer {


    public FramebufferBackBuffer(Graphics gfx) {
        super(gfx);
    }

    @Override
    protected void dispose(Graphics gfx) {
        //stub
    }
}
