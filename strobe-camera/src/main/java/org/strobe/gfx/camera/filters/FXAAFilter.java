package org.strobe.gfx.camera.filters;

import org.joml.Vector2f;
import org.strobe.gfx.Graphics;
import org.strobe.gfx.camera.CameraFilter;
import org.strobe.gfx.opengl.bindables.framebuffer.Framebuffer;
import org.strobe.gfx.opengl.bindables.mapper.Uniform;
import org.strobe.gfx.opengl.bindables.shader.Shader;
import org.strobe.gfx.opengl.bindables.shader.ShaderUniform;
import org.strobe.gfx.opengl.bindables.shader.io.ShaderLoader;
import org.strobe.gfx.opengl.bindables.texture.Texture2D;

public final class FXAAFilter implements CameraFilter {

    private final Shader shader;

    private final ShaderUniform<Texture2D> sourceTex;
    private final ShaderUniform<Vector2f> texelSize;

    public FXAAFilter(Graphics gfx){
        shader = ShaderLoader.loadShader(gfx, "filters/fxaa/");
        sourceTex = shader.getUniform(Texture2D.class, "source");
        texelSize = shader.getUniform(Vector2f.class, "texelSize");
    }

    @Override
    public void beforeFSR(Graphics gfx, Framebuffer source) {
        Texture2D color0 = source.getAttachmentTexture(Framebuffer.Attachment.COLOR_RGBA_ATTACHMENT_0);
        gfx.bind(shader);
        this.sourceTex.set(gfx, color0);
        texelSize.set(gfx, new Vector2f(1.0f/color0.getWidth(), 1.0f/color0.getHeight()));
    }

    @Override
    public void afterFSR(Graphics gfx) {
        gfx.unbind(shader);
    }


}
