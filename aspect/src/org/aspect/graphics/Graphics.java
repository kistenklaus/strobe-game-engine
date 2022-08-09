package org.aspect.graphics;

import org.aspect.exceptions.AspectGraphicsException;
import org.aspect.graphics.bindable.*;
import org.aspect.graphics.bindable.shader.ShaderIndexBuffer;
import org.aspect.graphics.bindable.shader.ShaderTexture;
import org.aspect.graphics.renderer.Renderer;
import org.aspect.graphics.window.Window;

import java.nio.ByteBuffer;

public class Graphics {

    private GraphicsAPI graphicsAPI = null;

    public Renderer createRenderer(Window window) {
        return new Renderer(window, this);
    }

    public void setGraphicsAPI(String graphicsAPI){
        this.graphicsAPI = GraphicsAPI.valueOf(graphicsAPI);
    }

    public void drawElements(int mode, int drawCount, int format){
        graphicsAPI.elementDrawCall(mode, drawCount, format);
    }

    public int getTriangleMode(){
        return graphicsAPI.getTriangleMode();
    }

    public int getLineMode(){
        return graphicsAPI.getLineMode();
    }

    public int getDefaultInternalFormat(){
        return graphicsAPI.getDefaultInternalFormat();
    }

    public String getFullScreenShaderPath(){
        return graphicsAPI.getFullScreenShaderPath();
    }

    public Framebuffer createFramebuffer(int width, int height){
        if(graphicsAPI == null)throw new AspectGraphicsException("not graphics API defined");
        Framebuffer framebuffer = graphicsAPI.newFramebuffer();
        framebuffer.create(width, height);
        return framebuffer;
    }

    public IndexBuffer createIndexBuffer(int[] content, int allocated_size, boolean readonly) {
        if(graphicsAPI == null)throw new AspectGraphicsException("not graphics API defined");
        IndexBuffer ibo = graphicsAPI.newIndexBuffer();
        ibo.create(content, allocated_size, readonly);
        return ibo;
    }

    public ShaderIndexBuffer wrapIndexBuffer(IndexBuffer ibo){
        ShaderIndexBuffer shaderIbo = graphicsAPI.newShaderIndexBuffer();
        shaderIbo.create(ibo);
        return shaderIbo;
    }

    public Shader createShader(String vertexSrc, String fragmentSrc){
        if(graphicsAPI == null)throw new AspectGraphicsException("not graphics API defined");
        Shader shader = graphicsAPI.newShader();
        shader.create(vertexSrc, fragmentSrc);
        return shader;
    }

    public Texture createTexture(ByteBuffer pixels, int width, int height,
                                 int channels, int samples, boolean interpolation){
        if(graphicsAPI == null)throw new AspectGraphicsException("not graphics API defined");
        Texture tex = graphicsAPI.newTexture();
        tex.create(pixels, width, height, channels, samples, interpolation);
        return tex;
    }

    public ShaderTexture wrapShaderTexture(Shader shader, Texture texture, String samplerName) {
        ShaderTexture shaderTexture = graphicsAPI.newShaderTexture();
        shaderTexture.create(shader, texture, samplerName);
        return shaderTexture;
    }

    public VertexArray createVertexArray(){
        if(graphicsAPI == null)throw new AspectGraphicsException("not graphics API defined");
        VertexArray vao = graphicsAPI.newVertexArray();
        vao.create();
        return vao;
    }

    public VertexBuffer createVertexBuffer(float[] content, int allocated_size, boolean readonly){
        if(graphicsAPI == null)throw new AspectGraphicsException("not graphics API defined");
        VertexBuffer vbo = graphicsAPI.newVertexBuffer();
        vbo.create(content, allocated_size, readonly);
        return vbo;
    }

    public boolean hasAPI() {
        return graphicsAPI != null;
    }

}
