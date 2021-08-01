package org.strobe.gfx.opengl.bindables.texture;

import org.lwjgl.opengl.GL11;

import static org.lwjgl.opengl.GL11.*;

public final class TextureOptions {

    private static final int DEFAULT_SAMPLES = 1;
    private static final int DEFAULT_MIN_FILTER = GL_LINEAR;
    private static final int DEFAULT_MAG_FILTER = GL_NEAREST;
    private static final int DEFAULT_WRAP_S = GL_REPEAT;
    private static final int DEFAULT_WRAP_T = GL_REPEAT;

    private int samples;
    private int min_filter;
    private int mag_filter;
    private int wrap_s;
    private int wrap_t;
    private TextureFormat format;

    public TextureOptions(int samples, int min_filter, int mag_filter, int wrap_s, int wrap_t, TextureFormat format) {
        this.samples = samples;
        this.min_filter = min_filter;
        this.mag_filter = mag_filter;
        this.wrap_s = wrap_s;
        this.wrap_t = wrap_t;
        this.format = format;
    }

    public TextureOptions(int samples, int min_filter, int mag_filter, TextureFormat format) {
        this.samples = samples;
        this.min_filter = min_filter;
        this.mag_filter = mag_filter;
        this.wrap_s = DEFAULT_WRAP_S;
        this.wrap_t = DEFAULT_WRAP_T;
        this.format = format;
    }

    public TextureOptions(int samples, TextureFormat format) {
        this.samples = samples;
        this.min_filter = DEFAULT_MIN_FILTER;
        this.mag_filter = DEFAULT_MAG_FILTER;
        this.wrap_s = DEFAULT_WRAP_S;
        this.wrap_t = DEFAULT_WRAP_T;
        this.format = format;
    }

    public TextureOptions(TextureFormat format){
        this.samples = DEFAULT_SAMPLES;
        this.min_filter = DEFAULT_MIN_FILTER;
        this.mag_filter = DEFAULT_MAG_FILTER;
        this.wrap_s = DEFAULT_WRAP_S;
        this.wrap_t = DEFAULT_WRAP_T;
        this.format = format;
    }

    public int getSamples() {
        return samples;
    }

    public int getMinFilter() {
        return min_filter;
    }

    public int getMagFilter() {
        return mag_filter;
    }

    public int getWrapS() {
        return wrap_s;
    }

    public int getWrapT() {
        return wrap_t;
    }

    public TextureFormat getFormat() {
        return format;
    }
}
