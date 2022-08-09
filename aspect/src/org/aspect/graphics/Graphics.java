package org.aspect.graphics;

import org.aspect.graphics.bindable.*;
import org.aspect.graphics.renderer.Renderer;
import org.aspect.window.Window;

import java.nio.ByteBuffer;

public class Graphics {

    public Renderer createRenderer(Window window) {
        return new Renderer(window, this);
    }

    public Framebuffer createFramebuffer(int width, int height) {
        Framebuffer framebuffer = new Framebuffer(width, height);
        framebuffer.create();
        return framebuffer;
    }

    public IndexBuffer createIndexBuffer(int[] content, int allocated_ints, boolean readonly) {
        IndexBuffer ibo = new IndexBuffer(content, allocated_ints, readonly);
        ibo.create();
        return ibo;
    }

    public Shader createShader(String vertexSrc, String fragmentSrc) {
        Shader shader = new Shader(vertexSrc, fragmentSrc);
        shader.create();
        return shader;
    }

    public Texture createTexture(ByteBuffer pixels, int width, int height,
                                 int samples, boolean interpolation,
                                 Texture.Format format, Texture.Type type) {
        Texture tex = new Texture(pixels, width, height,
                samples, interpolation, format, type);
        tex.create();
        return tex;
    }

    public VertexArray createVertexArray() {
        VertexArray vao = new VertexArray();
        vao.create();
        return vao;
    }

    public VertexBuffer createVertexBuffer(float[] content, int allocated_floats, boolean readonly) {
        VertexBuffer vbo = new VertexBuffer(content, allocated_floats, readonly);
        vbo.create();
        return vbo;
    }

    public UniformBuffer createUniformBuffer(int bindingIndex, String[] layout) {
        UniformBuffer ubo = new UniformBuffer(bindingIndex, layout);
        ubo.create();
        return ubo;
    }
}
