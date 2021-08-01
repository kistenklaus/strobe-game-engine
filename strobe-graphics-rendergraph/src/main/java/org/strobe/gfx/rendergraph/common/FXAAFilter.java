package org.strobe.gfx.rendergraph.common;

import org.joml.Vector2f;
import org.strobe.gfx.Graphics;
import org.strobe.gfx.opengl.bindables.framebuffer.Framebuffer;
import org.strobe.gfx.opengl.bindables.mapper.Uniform;
import org.strobe.gfx.opengl.bindables.shader.Shader;
import org.strobe.gfx.opengl.bindables.shader.Texture2DShaderUniform;
import org.strobe.gfx.opengl.bindables.shader.io.ShaderLoader;
import org.strobe.gfx.opengl.bindables.texture.Texture2D;

public final class FXAAFilter implements PostProcessingFilter{

    private static final String SHADER_DIRECTORY = "shaders/fxaa/";
    private static final String SOURCE_UNIFORM = "source";
    private static final String TEXEL_SIZE_UNIFORM = "texelSize";

    private final Shader shader;
    private final Uniform<Texture2D> sourceUniform;
    private final Uniform<Vector2f> texelSizeUniform;

    public FXAAFilter(Graphics gfx){
        this.shader = ShaderLoader.loadShader(gfx, SHADER_DIRECTORY);
        sourceUniform = shader.getUniform(Texture2D.class, SOURCE_UNIFORM);
        texelSizeUniform = shader.getUniform(Vector2f.class, TEXEL_SIZE_UNIFORM);
    }

    @Override
    public void prepareFSR(Graphics gfx, Framebuffer source) {
        Texture2D color0 ;
        if(source.hasAttachmentTextured(Framebuffer.Attachment.COLOR_RGB_ATTACHMENT_0))
            color0 = source.getAttachmentTexture(Framebuffer.Attachment.COLOR_RGB_ATTACHMENT_0);
        else if(source.hasAttachmentTextured(Framebuffer.Attachment.COLOR_RGBA_ATTACHMENT_0))
            color0 = source.getAttachmentTexture(Framebuffer.Attachment.COLOR_RGBA_ATTACHMENT_0);
        else throw new IllegalStateException();
        sourceUniform.set(gfx, color0);
        texelSizeUniform.set(gfx, new Vector2f(1.0f/color0.getWidth(), 1.0f/color0.getHeight()));
    }
}
