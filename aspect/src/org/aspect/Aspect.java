package org.aspect;

import org.aspect.exceptions.AspectGraphicsException;
import org.aspect.graphics.Graphics;
import org.aspect.graphics.bindable.*;
import org.aspect.graphics.primitives.PrimitiveIboContent;
import org.aspect.graphics.primitives.PrimitiveVboContent;
import org.aspect.window.Window;
import org.aspect.window.WindowAPI;
import org.aspect.io.AspectFileReader;
import org.joml.Vector3i;

import java.nio.ByteBuffer;

public class Aspect {

    private static Aspect instance;
    private static final String PLACEHOLDER_TEX_LOC = "assets/resources/none.png";

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
                                        int samples, boolean interpolation,
                                        Texture.Format format, Texture.Type type) {
        return getInstance().graphics.createTexture(pixels, width, height,
                samples, interpolation, format, type);
    }

    public static Texture loadTexture(String path, boolean interpolation) {
        final Vector3i specs = new Vector3i();
        final ByteBuffer pixels = AspectFileReader.loadImage(path, specs);
        Texture.Format format;
        if (specs.z == 3) format = Texture.Format.RGB;
        else if (specs.z == 4) format = Texture.Format.RGBA;
        else throw new AspectGraphicsException("can't read images with less than 3 or more than 4 channels");
        final Texture texture = createTexture(pixels, specs.x, specs.y,
                1, interpolation,
                format, Texture.Type.UNSIGNED_BYTE);
        AspectFileReader.freeImage(pixels);
        return texture;
    }

    /*
    ---------VERTEX-ARRAY-------------
     */
    public static VertexArray createVertexArray() {
        return getInstance().graphics.createVertexArray();
    }

    public static VertexArray createVertexArray(boolean readonlyIbo,
                                                PrimitiveIboContent ibo, boolean readonlyVbos, PrimitiveVboContent... vbos) {
        VertexArray vao = createVertexArray();
        if (ibo != null) {
            IndexBuffer indexBuffer = createIndexBuffer(ibo, readonlyIbo);
            vao.setIndexBuffer(indexBuffer);
        }
        for (int i = 0; i < vbos.length; i++) {
            VertexBuffer vertexBuffer = createVertexBuffer(vbos[i], readonlyVbos);
            vao.setAttribute(vertexBuffer,
                    i, vbos[i].getAttributeSize(),
                    vbos[i].getAttributeStride(), 1, 0);
        }
        return vao;
    }

    /*
    ---------VERTEX-BUFFER----------
     */
    public static VertexBuffer createVertexBuffer(float[] content, int allocated_size, boolean readonly) {
        return getInstance().graphics.createVertexBuffer(content, allocated_size, readonly);
    }

    public static VertexBuffer createVertexBuffer(PrimitiveVboContent primitive, boolean readonly) {
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

    public static IndexBuffer createIndexBuffer(PrimitiveIboContent primitive, boolean readonly) {
        int[] content = primitive.getContent();
        int allocatedSize = content.length;
        return createIndexBuffer(content, allocatedSize, readonly);
    }

    /*
    --------UNIFORM-BUFFER---------
     */

    public static UniformBuffer createUniformBuffer(int bindingIndex, String... layout) {
        return getInstance().graphics.createUniformBuffer(bindingIndex, layout);
    }

    public static Texture getSpaceHolderTex() {
        return Aspect.loadTexture(PLACEHOLDER_TEX_LOC, false);
    }


}
