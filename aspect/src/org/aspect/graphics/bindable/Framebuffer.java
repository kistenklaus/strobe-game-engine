package org.aspect.graphics.bindable;

import org.aspect.exceptions.AspectGraphicsException;

public abstract class Framebuffer extends Bindable {

    protected int width, height;

    public void create(int width, int height){
        this.width = width;
        this.height = height;
        createFramebuffer(width, height);
    }

    protected abstract void createFramebuffer(int width, int height);

    public void setColorAttachment(int colorAttachment, Texture texture){
        if(width != texture.getWidth() || height != texture.getHeight())
            throw new AspectGraphicsException("can't set a color attachment of a framebuffer " +
                    "when the dimensions do not match");
        attachColorAttachment(colorAttachment, texture);
    }

    protected abstract void attachColorAttachment(int colorAttachment, Texture texture);

    public void setDepth24Stencil8Attachment(int samples){
        attachDepth24Stencil8Attachment(samples);
    }

    protected abstract void attachDepth24Stencil8Attachment(int samples);

    public abstract void complete();

    public int getWidth(){
        return width;
    }

    public int getHeight(){
        return height;
    }


    public abstract int getID();
}
