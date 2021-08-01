package org.strobe.gfx.rendergraph.common;

import org.strobe.gfx.Graphics;
import org.strobe.gfx.opengl.bindables.framebuffer.Framebuffer;
import org.strobe.gfx.rendergraph.core.RenderPass;
import org.strobe.gfx.rendergraph.core.Resource;

public class BlitFramebufferPass extends RenderPass {

    private final int[] masks;
    private final Resource<Framebuffer> target;
    private final Resource<Framebuffer> source;

    public BlitFramebufferPass(int...masks){
        this.masks = masks;
        target = registerResource(Framebuffer.class, "target");
        source = registerResource(Framebuffer.class, "source");
    }

    @Override
    public void complete(Graphics gfx) {
        //stub
    }

    @Override
    protected void reset(Graphics gfx) {
        //stub
    }

    @Override
    public void render(Graphics gfx) {
        for(int mask : masks){
            source.get().copyTo(gfx, target.get(), mask);
        }
    }

    @Override
    public void dispose(Graphics gfx) {
        //stub
    }

    public Resource<Framebuffer> getTargetResource(){
        return target;
    }

    public Resource<Framebuffer> getSourceResource(){
        return source;
    }
}
