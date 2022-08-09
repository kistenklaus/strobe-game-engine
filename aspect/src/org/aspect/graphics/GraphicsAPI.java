package org.aspect.graphics;

import org.aspect.exceptions.AspectSupportException;
import org.aspect.graphics.bindable.*;
import org.aspect.graphics.api.opengl.*;
import org.aspect.graphics.bindable.shader.ShaderIndexBuffer;
import org.aspect.graphics.bindable.shader.ShaderTexture;
import org.aspect.graphics.renderer.drawcall.ElementDrawCall;

import java.util.function.Supplier;

import static org.lwjgl.opengl.GL11.*;

public enum GraphicsAPI {
    OPENGL(GLFramebuffer::new,
            GLIndexBuffer::new,
            GLShader::new,
            GLTexture::new,
            GLVertexArray::new,
            GLVertexBuffer::new,
            GLShaderTexture::new,
            ShaderIndexBuffer::new,
            (m, dc, f) -> glDrawElements(m, dc, f, 0),
            GL_TRIANGLES,
            GL_LINES,
            GL_UNSIGNED_INT,
            "/org/aspect/graphics/api/opengl/defaultshaders/fullScreen");

    private final Supplier<Framebuffer> framebufferSupplier;
    private final Supplier<IndexBuffer> indexBufferSupplier;
    private final Supplier<Shader> shaderSupplier;
    private final Supplier<Texture> textureSupplier;
    private final Supplier<VertexArray> vertexArraySupplier;
    private final Supplier<VertexBuffer> vertexBufferSupplier;

    private final Supplier<ShaderTexture> shaderTextureSupplier;
    private final Supplier<ShaderIndexBuffer> shaderIndexBufferSupplier;

    private final ElementDrawCall elementDrawCall;

    private final int triangleMode;
    private final int lineMode;

    private final String fullScreenShaderPath;

    private final int defaultInternalFormat;

    GraphicsAPI(Supplier<Framebuffer> framebufferSupplier,
                Supplier<IndexBuffer> indexBufferSupplier,
                Supplier<Shader> shaderSupplier,
                Supplier<Texture> textureSupplier,
                Supplier<VertexArray> vertexArraySupplier,
                Supplier<VertexBuffer> vertexBufferSupplier,
                Supplier<ShaderTexture> shaderTextureSupplier,
                Supplier<ShaderIndexBuffer> shaderIndexBufferSupplier,
                ElementDrawCall elementDrawCall,
                int triangleMode,
                int lineMode,
                int defaultInternalFormat,
                String fullScreenShaderPath) {

        this.framebufferSupplier = framebufferSupplier;
        this.indexBufferSupplier = indexBufferSupplier;
        this.shaderSupplier = shaderSupplier;
        this.vertexArraySupplier = vertexArraySupplier;
        this.vertexBufferSupplier = vertexBufferSupplier;
        this.textureSupplier = textureSupplier;

        this.shaderTextureSupplier = shaderTextureSupplier;
        this.shaderIndexBufferSupplier = shaderIndexBufferSupplier;

        this.elementDrawCall = elementDrawCall;

        this.triangleMode = triangleMode;
        this.lineMode = lineMode;
        this.defaultInternalFormat = defaultInternalFormat;

        this.fullScreenShaderPath = fullScreenShaderPath;
    }

    Framebuffer newFramebuffer() {
        if (framebufferSupplier == null) throw new AspectSupportException(name() + " does not support framebuffer");
        return framebufferSupplier.get();
    }

    Shader newShader() {
        if (framebufferSupplier == null) throw new AspectSupportException(name() + " does not support Shader");
        return shaderSupplier.get();
    }

    Texture newTexture() {
        if (textureSupplier == null) throw new AspectSupportException(name() + " does not support textures");
        return textureSupplier.get();
    }

    IndexBuffer newIndexBuffer() {
        if (framebufferSupplier == null) throw new AspectSupportException(name() + " does not support IndexBuffer");
        return indexBufferSupplier.get();
    }

    VertexArray newVertexArray() {
        if (framebufferSupplier == null) throw new AspectSupportException(name() + " does not support VertexArray");
        return vertexArraySupplier.get();
    }

    VertexBuffer newVertexBuffer() {
        if (framebufferSupplier == null) throw new AspectSupportException(name() + " does not support VertexBuffer");
        return vertexBufferSupplier.get();
    }

    ShaderTexture newShaderTexture() {
        return shaderTextureSupplier.get();
    }
    ShaderIndexBuffer newShaderIndexBuffer(){
        return shaderIndexBufferSupplier.get();
    }

    void elementDrawCall(int mode, int drawCount, int format) {
        elementDrawCall.drawCall(mode, drawCount, format);
    }

    public int getTriangleMode() {
        return triangleMode;
    }

    public int getDefaultInternalFormat() {
        return defaultInternalFormat;
    }

    public int getLineMode() {
        return lineMode;
    }

    public String getFullScreenShaderPath(){
        return fullScreenShaderPath;
    }


}
