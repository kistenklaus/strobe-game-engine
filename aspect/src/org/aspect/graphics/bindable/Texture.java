package org.aspect.graphics.bindable;

import org.aspect.exceptions.AspectGraphicsException;
import org.aspect.graphics.drawable.attachments.TextureAttachment;

import java.nio.ByteBuffer;

import static org.lwjgl.opengl.GL11.*;
import static org.lwjgl.opengl.GL30.GL_RGB16F;
import static org.lwjgl.opengl.GL30.GL_RGBA16F;
import static org.lwjgl.opengl.GL32.GL_TEXTURE_2D_MULTISAMPLE;
import static org.lwjgl.opengl.GL32.glTexImage2DMultisample;
import static org.lwjgl.system.MemoryUtil.NULL;

public class Texture extends Bindable {

    //should not be a field should only be accessible int create.
    private final ByteBuffer pixels;
    private final String hash;
    private int ID;
    private int width, height;
    private int samples;
    private boolean interpolation;
    private Format format;
    private Type type;
    private int texType;
    private boolean isDepth;

    public Texture(ByteBuffer pixels, int width, int height, int samples,
                   boolean interpolation, Format format, Type type) {
        StringBuilder hashStr = new StringBuilder();
        //to pixel data in the hash allows duplicated graphics memory

        hashStr.append(width).append(height).append(samples)
                .append(interpolation).append(format).append(type);
        hash = hashStr.toString();
        this.pixels = pixels;//that's just bad because the bytebuffer gets freed before the texture is disposed
        this.width = width;
        this.height = height;
        this.samples = samples;
        this.interpolation = interpolation;
        this.format = format;
        this.type = type;
    }



    public void create() {
        texType = samples > 1 ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;

        ID = glGenTextures();

        glBindTexture(texType, ID);

        glTexParameteri(texType,
                GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(texType,
                GL_TEXTURE_MAG_FILTER, interpolation ? GL_LINEAR : GL_NEAREST);

        glTexParameteri(texType, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(texType, GL_TEXTURE_WRAP_T, GL_REPEAT);

        if (samples > 1) {
            if (pixels == null) {
                glTexImage2DMultisample(texType, samples, format.internalFormat,
                        width, height, true);
            } else {
                throw new AspectGraphicsException("textures with defined with pixels can't be multisampled");
            }
        } else {
            if (pixels == null) {
                glTexImage2D(texType, 0, format.internalFormat, width, height,
                        0, format.format, type.type, NULL);
            } else {
                glTexImage2D(texType, 0, format.internalFormat,
                        width, height, 0, format.format,
                        type.type, pixels);
            }
        }
        glBindTexture(texType, 0);

    }

    @Override
    public void bind() {
        glBindTexture(texType, ID);
    }

    @Override
    public void unbind() {
        glBindTexture(texType, 0);
    }

    @Override
    public void dispose() {
        glDeleteTextures(ID);
    }

    public int getTexType() {
        return texType;
    }

    public int getID() {
        return ID;
    }

    public int getWidth() {
        return width;
    }

    public int getHeight() {
        return height;
    }

    public int getChannels() {
        return format.channels;
    }

    public int getMultisamples() {
        return samples;
    }

    public boolean isMultisampled() {
        return samples > 1;
    }

    public boolean isInterpolation() {
        return interpolation;
    }

    public String hash() {
        return hash;
    }

    public TextureAttachment makeAttachment(Shader shader, String samplerName) {
        return new TextureAttachment(this, shader, samplerName);
    }

    public enum Format {
        RGB(3, GL_RGB, GL_RGB),
        RGBA(4, GL_RGBA, GL_RGBA),
        RGB16F(3, GL_RGB16F, GL_RGB),
        RGBA16F(4, GL_RGBA16F, GL_RGBA),
        DEPTH(1, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT);

        private final int channels;
        private final int internalFormat;
        private final int format;

        Format(int channels, int internalFormat, int format) {
            this.channels = channels;
            this.internalFormat = internalFormat;
            this.format = format;
        }
    }

    public enum Type {
        UNSIGNED_BYTE(GL_UNSIGNED_BYTE), FLOAT(GL_FLOAT);
        private final int type;

        Type(int type) {
            this.type = type;
        }
    }


}
