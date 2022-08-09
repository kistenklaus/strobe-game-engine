package org.aspect.graphics.bindable;

import org.aspect.Aspect;
import org.aspect.datatype.Array;
import org.aspect.exceptions.AspectGraphicsException;

import static org.lwjgl.opengl.GL30.*;
import static org.lwjgl.opengl.GL32.GL_TEXTURE_2D_MULTISAMPLE;

public class Framebuffer extends Bindable {

    private static int hashAcc = 0;
    private final String hash;
    private int width, height;
    private int ID;
    private Array<Integer> drawBuffers = new Array<>(1);

    private Array<Texture> attachments = new Array<>();

    private Texture depthMap = null;

    public Framebuffer(int width, int height) {
        this.width = width;
        this.height = height;
        hash = "Framebuffer:" + hashAcc++;
    }

    public final void create() {
        ID = glGenFramebuffers();
    }

    public void attachColorAttachment(int colorAttachment, Texture texture, boolean drawBuffer) {

        if (drawBuffer) drawBuffers.add(colorAttachment);

        if (width != texture.getWidth() || height != texture.getHeight())
            throw new AspectGraphicsException("can't set a color attachment of a framebuffer " +
                    "when the dimensions do not match");
        if (!(texture instanceof Texture))
            throw new AspectGraphicsException("can't create an opengl " +
                    "Framebuffer with a non opengl Texture");

        final int texType = texture.getTexType();
        final int texID = texture.getID();

        glBindFramebuffer(GL_FRAMEBUFFER, ID);
        glFramebufferTexture2D(GL_FRAMEBUFFER,
                GL_COLOR_ATTACHMENT0 + colorAttachment,
                texType, texID, 0);
        glBindTexture(GL_TEXTURE_2D, texID);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        attachments.set(colorAttachment, texture);
    }

    public Texture attachDepthTexture() {
        depthMap = Aspect.createTexture(null, width, height, 1, false,
                Texture.Format.DEPTH, Texture.Type.FLOAT);


        glBindFramebuffer(GL_FRAMEBUFFER, ID);
        int type = depthMap.isMultisampled() ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, type, depthMap.getID(), 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return depthMap;
    }

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

    public void complete() {
        glBindFramebuffer(GL_FRAMEBUFFER, ID);
        int[] attachments = new int[drawBuffers.size()];
        for (int i = 0; i < attachments.length; i++)
            attachments[i] = GL_COLOR_ATTACHMENT0 + drawBuffers.get(i);
        glDrawBuffers(attachments);

        int status;
        if ((status = glCheckFramebufferStatus(GL_FRAMEBUFFER))
                != GL_FRAMEBUFFER_COMPLETE) {
            throw new AspectGraphicsException("the framebuffer is not complete : status = " + status);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

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

    public int getWidth() {
        return width;
    }

    public int getHeight() {
        return height;
    }

    public Texture getColorAttachment(int colorAttachment) {
        Texture tex = attachments.get(colorAttachment);
        if (tex == null) System.out.println("\"\\u001B[91m\"fetching a not existing color attachment\\u001B[0m\"");
        return tex;
    }

    public String hash() {
        return hash;
    }

    public boolean hasDepthMap(){
        return depthMap!=null;
    }

    public Texture getDepthMap(){
        return depthMap;
    }
}
