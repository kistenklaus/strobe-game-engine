package org.strobe.gfx.opengl.bindables.texture;

import static org.lwjgl.opengl.GL11.*;
import static org.lwjgl.opengl.GL11.GL_DEPTH_COMPONENT;
import static org.lwjgl.opengl.GL30.*;

public enum TextureFormat {
    RGB_UBYTE(3, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE),
    RGBA_UBYTE(4, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE),
    RGB16F_UBYTE(3, GL_RGB16F, GL_RGB, GL_UNSIGNED_BYTE),
    RGBA16F_UBYTE(4, GL_RGBA16F, GL_RGBA, GL_UNSIGNED_BYTE),
    DEPTH_FLOAT(1, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT),
    STENCIL_INDEX8_FLOAT(1, GL_STENCIL_INDEX8, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE),
    RED_UBYTE(1, GL_RED, GL_RED, GL_UNSIGNED_BYTE);

    private final int type;
    private final int channels;
    private final int internalFormat;
    private final int format;

    TextureFormat(int channels, int internalFormat, int format, int type) {
        this.channels = channels;
        this.internalFormat = internalFormat;
        this.format = format;
        this.type = type;
    }

    public int getChannels() {
        return channels;
    }

    public int getInternalFormat() {
        return internalFormat;
    }

    public int getFormat() {
        return format;
    }

    public int getType() {
        return type;
    }
}
