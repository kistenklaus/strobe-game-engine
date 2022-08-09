package org.aspect.graphics.api.opengl;

import org.aspect.exceptions.AspectGraphicsException;
import org.aspect.graphics.bindable.Framebuffer;
import org.aspect.graphics.bindable.Texture;

import static org.lwjgl.opengl.GL30.*;

public final class GLFramebuffer extends Framebuffer {

    private int ID;

    @Override
    public void createFramebuffer(int width, int height) {
        ID = glGenFramebuffers();
    }

    @Override
    public void attachColorAttachment(int colorAttachment, Texture texture) {
        if (!(texture instanceof GLTexture))
            throw new AspectGraphicsException("can't create an opengl " +
                    "Framebuffer with a non opengl Texture");
        GLTexture glTexture = (GLTexture) texture;

        final int texType = glTexture.getTexType();
        final int texID = glTexture.getID();

        glBindFramebuffer(GL_FRAMEBUFFER, ID);
        glFramebufferTexture2D(GL_FRAMEBUFFER,
                GL_COLOR_ATTACHMENT0 + colorAttachment,
                texType, texID, 0);
        glBindTexture(GL_TEXTURE_2D, texID);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    @Override
    public void attachDepth24Stencil8Attachment(int samples) {
        final int rboID = glGenRenderbuffers();

        glBindRenderbuffer(GL_RENDERBUFFER, rboID);
        if (samples > 1) {
            glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples,
                    GL_DEPTH24_STENCIL8, width, height);
        } else {
            glRenderbufferStorage(GL_RENDERBUFFER,
                    GL_DEPTH24_STENCIL8, width, height);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, ID);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                GL_DEPTH_STENCIL_ATTACHMENT,
                GL_RENDERBUFFER, rboID);

        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    @Override
    public void complete() {
        glBindFramebuffer(GL_FRAMEBUFFER, ID);
        int status;
        if ((status = glCheckFramebufferStatus(GL_FRAMEBUFFER))
                != GL_FRAMEBUFFER_COMPLETE) {
            throw new AspectGraphicsException("the framebuffer is not complete : status = " + status);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    @Override
    public int getID() {
        return ID;
    }

    @Override
    public void bind() {
        glBindFramebuffer(GL_FRAMEBUFFER, ID);
    }

    @Override
    public void unbind() {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    @Override
    public void dispose() {
        glDeleteFramebuffers(ID);
    }
}
