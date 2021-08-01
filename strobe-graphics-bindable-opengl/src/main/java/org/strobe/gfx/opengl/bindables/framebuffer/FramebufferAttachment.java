package org.strobe.gfx.opengl.bindables.framebuffer;

import org.strobe.debug.Debuggable;
import org.strobe.gfx.Graphics;
import org.strobe.gfx.opengl.bindables.texture.Texture2D;

public abstract class FramebufferAttachment implements Debuggable {

    private final boolean isTexture;
    private final boolean hasDepth;
    private final boolean hasStencil;

    FramebufferAttachment(boolean isTextured, boolean hasDepth, boolean hasStencil){
            this.isTexture = isTextured;
            this.hasDepth = hasDepth;
            this.hasStencil = hasStencil;
    }

    abstract void attach(Graphics gfx, Framebuffer fbo);

    public Texture2D getTexture(){
        throw new UnsupportedOperationException();
    }

    public boolean isTextured() {
        return isTexture;
    }

    public boolean hasDepth() {
        return hasDepth;
    }

    public boolean hasStencil() {
        return hasStencil;
    }
}
