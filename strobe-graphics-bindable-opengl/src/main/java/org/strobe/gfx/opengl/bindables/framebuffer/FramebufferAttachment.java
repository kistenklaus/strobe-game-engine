package org.strobe.gfx.opengl.bindables.framebuffer;

import org.strobe.debug.Debuggable;
import org.strobe.gfx.Graphics;
import org.strobe.gfx.opengl.bindables.texture.Texture2D;

public abstract class FramebufferAttachment implements Debuggable {

    protected final int attachment;
    private final boolean isTexture;
    private final boolean isDrawBuffer;
    private final boolean hasDepth;
    private final boolean hasStencil;

    FramebufferAttachment(int attachment, boolean isTextured, boolean isDrawBuffer, boolean hasDepth, boolean hasStencil) {
        this.attachment = attachment;
        this.isTexture = isTextured;
        this.isDrawBuffer = isDrawBuffer;
        this.hasDepth = hasDepth;
        this.hasStencil = hasStencil;
    }

    abstract void attach(Graphics gfx, Framebuffer fbo);

    public Texture2D getTexture() {
        throw new UnsupportedOperationException();
    }

    public boolean isTextured() {
        return isTexture;
    }

    protected boolean isDrawBuffer() {
        return isDrawBuffer;
    }

    public boolean hasDepth() {
        return hasDepth;
    }

    public boolean hasStencil() {
        return hasStencil;
    }

    protected int getAttachment(){
        return attachment;
    }
}
