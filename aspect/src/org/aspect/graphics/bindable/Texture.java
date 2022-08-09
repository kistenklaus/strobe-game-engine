package org.aspect.graphics.bindable;

import java.nio.ByteBuffer;

public abstract class Texture extends Bindable {

    private int width, height;
    private int channels;
    private int samples;
    private boolean interpolation;

    public void create(ByteBuffer pixels, int width, int height,
                       int channels, int samples, boolean interpolation) {
        this.width = width;
        this.height = height;
        this.channels = channels;
        this.samples = samples;
        this.interpolation = interpolation;

        createTexture(pixels, width, height, channels, samples, interpolation);

    }

    protected abstract void createTexture(ByteBuffer pixels, int width, int height, int channels, int samples, boolean interpolation);


    public int getWidth() {
        return width;
    }

    public int getHeight() {
        return height;
    }

    public int getChannels() {
        return channels;
    }

    public int getMultisamples() {
        return samples;
    }

    public boolean isMultisampled() {
        return samples > 1;
    }

    public boolean isInterpolated() {
        return interpolation;
    }
}
