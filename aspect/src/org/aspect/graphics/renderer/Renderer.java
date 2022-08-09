package org.aspect.graphics.renderer;

import org.aspect.Aspect;
import org.aspect.exceptions.AspectGraphicsException;
import org.aspect.graphics.Graphics;
import org.aspect.graphics.bindable.*;
import org.aspect.graphics.api.opengl.GLFramebuffer;
import org.aspect.graphics.bindable.shader.ShaderTexture;
import org.aspect.graphics.primitive.Primitive;
import org.aspect.graphics.renderer.drawable.Drawable;
import org.aspect.graphics.renderer.graph.RenderGraph;
import org.aspect.graphics.renderer.graph.passes.Pass;
import org.aspect.graphics.window.Window;
import org.aspect.shaderio.AspectFileReader;

import static org.lwjgl.opengl.GL11.*;

public final class Renderer {

    private final Window window;

    private final RenderGraph graph = new RenderGraph(this);

    private final Graphics graphics;

    private Framebuffer back_buffer;
    private ShaderTexture back_buffer_texture;
    private final VertexArray screenVao;
    private final Shader fullScreenShader;

    private DrawMethod drawMethod;

    private int drawMode;
    private int drawCount;
    private int internalFormat;

    public Renderer(Window window, Graphics graphics) {
        if (!graphics.hasAPI())
            throw new AspectGraphicsException("no graphics api defined");

        this.window = window;
        this.graphics = graphics;
        this.internalFormat = graphics.getDefaultInternalFormat();

        this.back_buffer = graphics.createFramebuffer(window.getWidth(), window.getHeight());
        Texture back_buffer_texture = graphics.createTexture(null, window.getWidth(),
                window.getHeight(), 4, 1, false);
        this.back_buffer.setColorAttachment(0, back_buffer_texture);

        screenVao = graphics.createVertexArray();

        VertexBuffer positions = Aspect.createVertexBuffer(
                new Primitive.QuadPositions(2,2), true);

        VertexBuffer textureCords = Aspect.createVertexBuffer(
                new Primitive.QuadUVs(), true);

        IndexBuffer ibo = graphics.createIndexBuffer(new int[]{
                0, 1, 2,
                2, 3, 0
        }, 6, true);

        screenVao.setAttribute(positions, 0,
                Primitive.QuadPositions.size, Primitive.QuadPositions.stride,
                1, 0);

        screenVao.setAttribute(textureCords, 1,
                Primitive.QuadUVs.size, Primitive.QuadUVs.stride,
                1, 0);

        screenVao.setIndexBuffer(ibo);

        fullScreenShader = Aspect.loadShaderFromClasspath(
                graphics.getFullScreenShaderPath()+".vs",
                graphics.getFullScreenShaderPath()+".fs");

        this.back_buffer_texture = Aspect.wrapTexture(fullScreenShader,
                back_buffer_texture, "screen");

        glEnable(GL_TEXTURE_2D);

        resetDrawSettings();
    }

    public void executeDrawCall() {
        switch (drawMethod) {
            case ELEMENTS -> graphics.drawElements(drawMode, drawCount, internalFormat);
        }
    }

    public Framebuffer getBackBuffer() {
        return back_buffer;
    }

    public void resetDrawSettings() {
        this.drawMethod = DrawMethod.ELEMENTS;
        this.drawMode = graphics.getTriangleMode();
    }

    public void setDrawMode(DrawMode mode) {
        this.drawMode = mode.getAPIMode(graphics);
    }

    public void setDrawCount(int drawCount) {
        this.drawCount = drawCount;
    }

    /*
      ------------Render-Graph------------
     */
    public void attachPass(Pass pass) {
        graph.addPass(pass);
    }

    public void link() {
        graph.link();
    }

    public void complete() {
        graph.complete();
    }

    public void draw(Drawable drawable) {
        drawable.submit(graph);
    }

    public void render() {

        graph.render();

        back_buffer.unbind();

        fullScreenShader.bind();
        back_buffer_texture.bind();
        back_buffer_texture.prepare(this, fullScreenShader);

        screenVao.bind();
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        back_buffer_texture.unbind();
        fullScreenShader.unbind();


    }

    public <T> void registerGlobalResource(String name, Class<T> resourceClass, T resource) {
        graph.registerGlobalResource(name, resourceClass, resource);
    }

    public void setLinkage(String source, String sink) {
        graph.setLinkage(source, sink);
    }


}
