package org.aspect;

import org.aspect.graphics.Graphics;
import org.aspect.graphics.bindable.*;
import org.aspect.graphics.bindable.shader.ShaderIndexBuffer;
import org.aspect.graphics.bindable.shader.ShaderTexture;
import org.aspect.graphics.primitive.Primitive;
import org.aspect.graphics.window.Window;
import org.aspect.graphics.window.WindowAPI;
import org.aspect.shaderio.AspectFileReader;
import org.joml.Vector3i;

import java.nio.ByteBuffer;

public class Aspect {

    private static Aspect instance;

    private final Graphics graphics;

    private Aspect() {
        this.graphics = new Graphics();
    }

    private static Aspect getInstance() {
        if (instance == null) instance = new Aspect();
        return instance;
    }

    /*
    ----------WINDOW-----------
     */
    public static Window createWindow(String title, int width, int height,
                                      boolean fullscreen, WindowAPI api) {
        Window window = api.newWindow();
        window.initializeWindow(getInstance().graphics, title, width, height, fullscreen);
        return window;
    }

    public static void setGraphicsAPI(String graphicsAPI) {
        getInstance().graphics.setGraphicsAPI(graphicsAPI.toUpperCase());
    }


    /*
    ---------FRAMEBUFFER-----------
     */
    public static Framebuffer createFramebuffer(int width, int height) {
        return getInstance().graphics.createFramebuffer(width, height);
    }


    /*
    ----------SHADER-------
     */
    public static Shader createShader(String vertexSrc, String fragmentSrc) {
        return getInstance().graphics.createShader(vertexSrc, fragmentSrc);
    }

    public static Shader loadShader(String vsPath, String fsPath) {
        final String vsSrc = AspectFileReader.readFile(vsPath);
        final String fsSrc = AspectFileReader.readFile(fsPath);
        return createShader(vsSrc, fsSrc);
    }

    public static Shader loadShaderFromClasspath(String vsPath, String fsPath) {
        final String vsSrc = AspectFileReader.readFileFromClasspath(vsPath);
        final String fsSrc = AspectFileReader.readFileFromClasspath(fsPath);
        return createShader(vsSrc, fsSrc);
    }


    /*
     ----------TEXTURE----------------
     */
    public static Texture createTexture(ByteBuffer pixels, int width, int height,
                                        int channels, int samples, boolean interpolation) {
        return getInstance().graphics.createTexture(pixels, width, height,
                channels, samples, interpolation);
    }

    public static Texture loadTexture(String path, boolean interpolation) {
        final Vector3i specs = new Vector3i();
        final ByteBuffer pixels = AspectFileReader.loadImage(path, specs);
        final Texture texture = createTexture(pixels, specs.x, specs.y, specs.z,
                1, interpolation);
        AspectFileReader.freeImage(pixels);
        return texture;
    }


    public static ShaderTexture wrapTexture(Shader shader, Texture texture, String samplerName) {
        return getInstance().graphics.wrapShaderTexture(shader, texture, samplerName);
    }

    public static ShaderTexture createWrappedTexture(ByteBuffer pixels, int width, int height,
                                                     int channels, int samples, boolean interpolation,
                                                     Shader shader, String samplerName) {
        Texture texture = createTexture(pixels, width, height, channels, samples, interpolation);
        return wrapTexture(shader, texture, samplerName);
    }

    public static ShaderTexture loadWrappedTexture(String path, boolean interpolation,
                                                   Shader shader, String samplerName) {
        Texture texture = loadTexture(path, interpolation);
        return wrapTexture(shader, texture, samplerName);
    }

    /*
    ---------VERTEX-ARRAY-------------
     */
    public static VertexArray createVertexArray() {
        return getInstance().graphics.createVertexArray();
    }

    /*
    ---------VERTEX-BUFFER----------
     */
    public static VertexBuffer createVertexBuffer(float[] content, int allocated_size, boolean readonly) {
        return getInstance().graphics.createVertexBuffer(content, allocated_size, readonly);
    }

    public static VertexBuffer createVertexBuffer(Primitive primitive, boolean readonly){
        float[] content = primitive.getVboContext();
        int allocated_size = content.length;
        return createVertexBuffer(content, allocated_size, readonly);
    }

    /*
    ---------INDEX-BUFFER---------
    */
    public static IndexBuffer createIndexBuffer(int[] content, int allocated_size, boolean readonly) {
        return getInstance().graphics.createIndexBuffer(content, allocated_size, readonly);
    }

    public static ShaderIndexBuffer wrapIndexBuffer(IndexBuffer ibo) {
        return getInstance().graphics.wrapIndexBuffer(ibo);
    }

    public static ShaderIndexBuffer createWrappedIndexBuffer(int[] content, int allocated_size, boolean readonly) {
        IndexBuffer ibo = createIndexBuffer(content, allocated_size, readonly);
        return wrapIndexBuffer(ibo);
    }
}
