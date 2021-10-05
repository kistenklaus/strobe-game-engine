package org.strobe.gfx.opengl.bindables.framebuffer;

import org.strobe.debug.Debug;
import org.strobe.debug.Debuggable;
import org.strobe.gfx.Bindable;
import org.strobe.gfx.Graphics;
import org.strobe.gfx.Pool;
import org.strobe.gfx.opengl.bindables.texture.Texture2D;
import org.strobe.gfx.opengl.bindables.texture.TextureFormat;
import org.strobe.gfx.opengl.bindables.texture.TextureOptions;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.function.Supplier;

import static org.lwjgl.opengl.GL30.*;

@Pool(FramebufferPool.class)
public class Framebuffer extends Bindable<FramebufferPool> implements Debuggable {

    private static Framebuffer backBuffer = null;
    public static Framebuffer getBackBuffer(Graphics gfx){
        if(backBuffer != null) return backBuffer;
        backBuffer = new FramebufferBackBuffer(gfx);
        return backBuffer;
    }

    private final int ID;
    private final int width;
    private final int height;
    @Debug
    private final FramebufferAttachment[] attachments;
    private final HashMap<Attachment, Integer> indexMap = new HashMap<>();
    private final boolean hasDepth;
    private final boolean hasStencil;

    public Framebuffer(Graphics gfx, int width, int height, Attachment... attachments) {
        super(gfx);
        this.width = width;
        this.height = height;
        ID = glGenFramebuffers();
        gfx.bind(this);
        this.attachments = new FramebufferAttachment[attachments.length];
        boolean tempDepth = false;
        boolean tempStencil = false;
        ArrayList<Integer> drawBuffers = new ArrayList<>();
        for (int i = 0; i < attachments.length; i++) {
            FramebufferAttachment att = attachments[i].get();
            tempDepth |= att.hasDepth();
            tempStencil |= att.hasStencil();
            this.attachments[i] = att;
            indexMap.put(attachments[i], i);
            att.attach(gfx, this);

            if(att.isDrawBuffer())drawBuffers.add(att.getAttachment());
        }
        this.hasDepth = tempDepth;
        this.hasStencil = tempStencil;

        if(drawBuffers.isEmpty()){
            glDrawBuffer(GL_NONE);
        }else{
            int[] bufs = new int[drawBuffers.size()];
            for(int i=0;i<drawBuffers.size();i++)bufs[i] = drawBuffers.get(i);
            glDrawBuffers(bufs);
        }

        int status;
        if ((status = glCheckFramebufferStatus(GL_FRAMEBUFFER)) != GL_FRAMEBUFFER_COMPLETE) {
            throw new RuntimeException("the fbo is not complete status=" + status);
        }
        pool.add(this);
        gfx.unbind(this);
    }

    protected Framebuffer(Graphics gfx){
        super(gfx);
        this.ID = 0;
        this.width = gfx.getWidth();
        this.height = gfx.getHeight();
        attachments = null;
        hasDepth = true;
        hasStencil = false;
    }

    @Override
    protected void bind(Graphics gfx){
        glViewport(0,0, width,height);
        glBindFramebuffer(GL_FRAMEBUFFER, ID);
    }

    @Override
    protected void unbind(Graphics gfx) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    @Override
    protected void dispose(Graphics gfx) {
        glDeleteFramebuffers(ID);
    }

    public void copyTo(Graphics gfx, Framebuffer target, int mask){
        copyTo(gfx, target, mask, GL_NEAREST);
    }

    public void copyTo(Graphics gfx, Framebuffer target, int mask, int filter) {
        int width;
        int height;
        int targetID;
        if (target == null) {
            width = gfx.getWidth();
            height = gfx.getHeight();
            targetID = 0;
        } else {
            width = target.getWidth();
            height = target.getHeight();
            targetID = target.ID;
        }
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, targetID);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, ID);
        glBlitFramebuffer(0, 0, this.width, this.height, 0, 0, width, height, mask, filter);
        pool.rebind(gfx);
    }

    public Iterable<Attachment> getAttachmentIterable() {
        return indexMap.keySet();
    }

    public Texture2D getAttachmentTexture(Attachment attachment) {
        Integer index = indexMap.get(attachment);
        if (index == null) throw new RuntimeException("framebuffer doesn't have an attachment : " + attachment.name());
        FramebufferAttachment att = attachments[index];
        if (!att.isTextured()) return null;
        return att.getTexture();
    }

    public boolean hasAttachmentTextured(Attachment attachment) {
        return indexMap.get(attachment) != null && getAttachmentTexture(attachment) != null;
    }

    public boolean hasDepth() {
        return hasDepth;
    }

    public boolean hasStencil() {
        return hasStencil;
    }

    public int getWidth() {
        return width;
    }

    public int getHeight() {
        return height;
    }

    public int getID() {
        return ID;
    }

    @Override
    public String toString() {
        return "Framebuffer{" +
                "ID=" + ID +
                ", width=" + width +
                ", height=" + height +
                ", hasDepth=" + hasDepth +
                ", hasStencil=" + hasStencil +
                '}';
    }

    public enum Attachment {
        COLOR_RGB_ATTACHMENT_0("color0", () -> new FramebufferTextureAttachment(GL_COLOR_ATTACHMENT0,
                new TextureOptions(1, GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, TextureFormat.RGB_UBYTE),
                true, false, false)),
        COLOR_RGBA_ATTACHMENT_0("color0", () -> new FramebufferTextureAttachment(GL_COLOR_ATTACHMENT0,
                new TextureOptions(1, GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, TextureFormat.RGBA_UBYTE),
                true, false, false)),
        COLOR_RGBA_ATTACHMENT_1("color1", () -> new FramebufferTextureAttachment(GL_COLOR_ATTACHMENT1,
                new TextureOptions(1, GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, TextureFormat.RGBA_UBYTE),
                true,false, false)),
        COLOR_RGBA_ATTACHMENT_2("color2", () -> new FramebufferTextureAttachment(GL_COLOR_ATTACHMENT2,
                new TextureOptions(1, GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, TextureFormat.RGBA_UBYTE),
                true, false, false)),
        COLOR_RGBA_ATTACHMENT_3("color3", () -> new FramebufferTextureAttachment(GL_COLOR_ATTACHMENT3,
                new TextureOptions(1, GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, TextureFormat.RGBA_UBYTE),
                true, false, false)),
        DEPTH_ATTACHMENT("depth", () -> new FramebufferTextureAttachment(GL_DEPTH_ATTACHMENT,
                new TextureOptions(1, GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, TextureFormat.DEPTH_FLOAT),
                false, true, false)),
        STENCIL_ATTACHMENT("stencil", ()->new FramebufferTextureAttachment(GL_STENCIL_ATTACHMENT,
                new TextureOptions(1, GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, TextureFormat.STENCIL_INDEX8_FLOAT),
                false, false, true)),
        DEPTH_RBO_ATTACHMENT("depthRbo", () -> new FramebufferRboAttachment(GL_DEPTH_COMPONENT,
                GL_DEPTH_ATTACHMENT, 1, true, false)),
        SHADOW_DEPTH_ATTACHMENT("shadowDepth", ()->new FramebufferShadowDepthAttachment());

        private final Supplier<FramebufferAttachment> constructor;
        private final String shortName;

        Attachment(String shortName, Supplier<FramebufferAttachment> constructor) {
            this.constructor = constructor;
            this.shortName = shortName;
        }

        FramebufferAttachment get() {
            return constructor.get();
        }

        public String shortName() {
            return shortName;
        }
    }
}
