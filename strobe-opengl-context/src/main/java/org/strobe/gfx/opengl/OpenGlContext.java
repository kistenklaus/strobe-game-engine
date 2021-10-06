package org.strobe.gfx.opengl;

import org.strobe.engine.StrobeContext;
import org.strobe.gfx.Graphics;
import org.strobe.gfx.opengl.bindables.framebuffer.Framebuffer;

public abstract class OpenGlContext extends StrobeContext {

    protected Framebuffer target = null;
    private final Framebuffer.Attachment[] targetAttachments;

    public OpenGlContext(String title, int width, int height, Framebuffer.Attachment... targetAttachments) {
        super(title, width, height);
        this.targetAttachments = targetAttachments;
    }

    @Override
    public void init(Graphics gfx) {
        target = new Framebuffer(gfx, getWidth(), getHeight(), targetAttachments);
    }

    public Framebuffer getTarget(){
        if(target == null)throw new IllegalArgumentException("target has not been initialized");
        return target;
    }

}
