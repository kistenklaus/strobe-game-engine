package org.strobe.gfx.opengl.bindables.framebuffer;

import org.strobe.debug.Debug;
import org.strobe.gfx.Graphics;

import static org.lwjgl.opengl.GL30.*;

class FramebufferRboAttachment extends FramebufferAttachment {

    private final int internalFormat;
    private int rboID;
    @Debug
    private final int samples;

    public FramebufferRboAttachment(int attachment, int internalFormat, int samples, boolean hasDepth, boolean hasStencil){
        super(attachment, false, false, hasDepth, hasStencil);
        this.internalFormat = internalFormat;
        this.samples = samples;
    }

    @Override
    public void attach(Graphics gfx, Framebuffer fbo) {
        rboID = glGenRenderbuffers();
        if(samples > 1){
            glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, internalFormat, fbo.getWidth(), fbo.getHeight());
        }else{
            glRenderbufferStorage(GL_RENDERBUFFER, internalFormat, fbo.getWidth(), fbo.getHeight());
        }
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, rboID);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
    }
}
