package org.strobe.gfx.rendergraph.common;

import org.strobe.gfx.Graphics;
import org.strobe.gfx.opengl.bindables.framebuffer.Framebuffer;
import org.strobe.gfx.rendergraph.core.BindingPass;
import org.strobe.gfx.rendergraph.core.Resource;

import static org.lwjgl.opengl.GL11.glClear;
import static org.lwjgl.opengl.GL11.glClearColor;
import static org.lwjgl.opengl.GL30.GL_FRAMEBUFFER;
import static org.lwjgl.opengl.GL30.glBindFramebuffer;

public final class ClearFramebufferPass extends BindingPass {

    private final int mask;
    private final Resource<Framebuffer> target;

    public ClearFramebufferPass(int mask){
        this.mask = mask;
        target = registerResource(Framebuffer.class, "target");
    }

    @Override
    public void complete(Graphics gfx) {
        addBindable(target.get());
    }

    @Override
    public void renderBoundBindables(Graphics gfx) {
        glClearColor(0,0,0,1);
        glClear(mask);
    }


    @Override
    public void dispose(Graphics gfx) {
        //stub
    }

    public Resource<Framebuffer> getTargetResource(){
        return target;
    }
}
