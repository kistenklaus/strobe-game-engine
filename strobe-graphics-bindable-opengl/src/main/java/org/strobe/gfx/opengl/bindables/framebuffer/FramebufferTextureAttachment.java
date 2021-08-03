package org.strobe.gfx.opengl.bindables.framebuffer;

import org.strobe.debug.Debug;
import org.strobe.gfx.Graphics;
import org.strobe.gfx.opengl.bindables.texture.Texture2D;
import org.strobe.gfx.opengl.bindables.texture.TextureOptions;

import static org.lwjgl.opengl.GL30.GL_FRAMEBUFFER;
import static org.lwjgl.opengl.GL30.glFramebufferTexture2D;

class FramebufferTextureAttachment extends FramebufferAttachment {

    private final TextureOptions options;
    @Debug
    private Texture2D tex;

    public FramebufferTextureAttachment(int attachment, TextureOptions textureOptions, boolean isDrawBuffer, boolean hasDepth, boolean hasStencil) {
        super(attachment, true, isDrawBuffer, hasDepth, hasStencil);
        this.options = textureOptions;
    }

    @Override
    public void attach(Graphics gfx, Framebuffer fbo) {
        tex = new Texture2D(gfx, fbo.getWidth(), fbo.getHeight(), options, null);
        glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, tex.getTarget(), tex.getID(), 0);
    }

    @Override
    public Texture2D getTexture(){
        return tex;
    }
}
