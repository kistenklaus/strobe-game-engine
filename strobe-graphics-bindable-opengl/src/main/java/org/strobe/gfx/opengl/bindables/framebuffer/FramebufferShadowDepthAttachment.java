package org.strobe.gfx.opengl.bindables.framebuffer;

import org.strobe.gfx.Graphics;
import org.strobe.gfx.opengl.bindables.texture.Texture2D;
import org.strobe.gfx.opengl.bindables.texture.TextureFormat;
import org.strobe.gfx.opengl.bindables.texture.TextureOptions;

import static org.lwjgl.opengl.GL11.*;
import static org.lwjgl.opengl.GL12.GL_CLAMP_TO_EDGE;
import static org.lwjgl.opengl.GL30.*;
import static org.lwjgl.opengl.GL30.GL_FRAMEBUFFER;

public class FramebufferShadowDepthAttachment extends FramebufferTextureAttachment{

    public FramebufferShadowDepthAttachment() {
        super(GL_DEPTH_ATTACHMENT, new TextureOptions(1, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, TextureFormat.DEPTH_FLOAT), false, true, false);
    }

    @Override
    public void attach(Graphics gfx, Framebuffer fbo) {
        tex = new Texture2D(gfx, fbo.getWidth(), fbo.getHeight(), options, null);
        //TODO saw something with parameteri GL_TEXTURE_COMPARE_MODE and sampler2DShadow
        glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, tex.getTarget(), tex.getID(), 0);
    }
}
