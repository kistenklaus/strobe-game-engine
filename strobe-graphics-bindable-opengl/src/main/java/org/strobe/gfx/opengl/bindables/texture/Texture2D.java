package org.strobe.gfx.opengl.bindables.texture;

import org.strobe.gfx.Bindable;
import org.strobe.gfx.Graphics;
import org.strobe.gfx.Pool;

import java.nio.ByteBuffer;

import static org.lwjgl.opengl.GL11.*;
import static org.lwjgl.opengl.GL32.GL_TEXTURE_2D_MULTISAMPLE;
import static org.lwjgl.opengl.GL32.glTexImage2DMultisample;

@Pool(TexturePool2D.class)
public class Texture2D extends Bindable<TexturePool2D> {

    private final int ID;

    private final int target;

    private final int width;
    private final int height;

    private final TextureOptions options;

    public Texture2D(Graphics gfx, TextureOptions options, TextureLoader.PixelBuffer pixels) {
        super(gfx);
        pool.add(this);
        this.width = pixels.getWidth();
        this.height = pixels.getHeight();
        this.options = options;
        this.target = options.getSamples() > 1 ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
        pixels.convertTo(options.getFormat().getChannels());
        ID = glGenTextures();

        createTexture(gfx, width, height, options.getMinFilter(), options.getMagFilter(),
                options.getWrapS(), options.getWrapT(), options.getFormat().getFormat(),
                options.getFormat().getInternalFormat(), options.getSamples(), options.getFormat().getType(), pixels.getPixelBuffer());
        pixels.dispose();
    }

    public Texture2D(Graphics gfx, int width, int height, TextureOptions options, ByteBuffer pixels) {
        super(gfx);
        pool.add(this);
        this.width = width;
        this.height = height;
        this.options = options;
        this.target = options.getSamples() > 1 ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
        ID = glGenTextures();
        createTexture(gfx, width, height, options.getMinFilter(), options.getMagFilter(),
                options.getWrapS(), options.getWrapT(), options.getFormat().getFormat(),
                options.getFormat().getInternalFormat(), options.getSamples(), options.getFormat().getType(), pixels);
    }

    private final void createTexture(Graphics gfx, int width, int height, int min_filter, int mag_filter,
                                     int wrap_s, int wrap_t, int format, int internal_format,
                                     int samples, int type, ByteBuffer pixels) {
        gfx.bind(this);
        glTexParameteri(target, GL_TEXTURE_MIN_FILTER, min_filter);
        glTexParameteri(target, GL_TEXTURE_MAG_FILTER, mag_filter);
        glTexParameteri(target, GL_TEXTURE_WRAP_S, wrap_s);
        glTexParameteri(target, GL_TEXTURE_WRAP_T, wrap_t);

        glPixelStorei(GL_PACK_ALIGNMENT, 1);

        if (target == GL_TEXTURE_2D_MULTISAMPLE) {
            if (pixels == null) {
                glTexImage2DMultisample(target, samples, internal_format,
                        width, height, true);
            } else {
                throw new UnsupportedOperationException("can't create multisampled textures based on pixel data");
            }
        } else {
            glTexImage2D(target, 0, internal_format, width, height,
                    0, format, type, pixels);
        }
        gfx.unbind(this);
    }

    @Override
    protected void bind(Graphics gfx) {
        glBindTexture(target, ID);
    }

    @Override
    protected void unbind(Graphics gfx) {
        glBindTexture(target, 0);
    }

    @Override
    protected void dispose(Graphics gfx) {
        glDeleteTextures(ID);
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

    public TextureFormat getFormat() {
        return options.getFormat();
    }

    public int getTarget() {
        return target;
    }

    public int getSamples() {
        return options.getSamples();
    }

    public boolean isMultisampled() {
        return options.getSamples() > 1;
    }
}
