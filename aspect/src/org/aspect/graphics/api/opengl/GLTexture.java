package org.aspect.graphics.api.opengl;

import org.aspect.exceptions.AspectGraphicsException;
import org.aspect.graphics.bindable.Shader;
import org.aspect.graphics.bindable.Texture;
import org.aspect.graphics.renderer.Renderer;

import java.nio.ByteBuffer;

import static org.lwjgl.opengl.GL11.*;
import static org.lwjgl.opengl.GL13.GL_TEXTURE0;
import static org.lwjgl.opengl.GL13.glActiveTexture;
import static org.lwjgl.opengl.GL32.GL_TEXTURE_2D_MULTISAMPLE;
import static org.lwjgl.opengl.GL32.glTexImage2DMultisample;
import static org.lwjgl.system.MemoryUtil.NULL;

public final class GLTexture extends Texture {

    private int ID;

    private int texType;

    @Override
    public void createTexture(ByteBuffer pixels, int width, int height,
                              int channels, int samples,
                              boolean interpolation) {

        texType = samples > 1 ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;

        int format;
        if (channels == 3) {
            format = GL_RGB;
        } else if (channels == 4) {
            format = GL_RGBA;
        } else {
            throw new AspectGraphicsException("textures can't be initialized with " + channels + " channels");
        }

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
                glTexImage2DMultisample(texType, samples, format,
                        width, height, true);
            } else {
                throw new AspectGraphicsException("textures with defined with pixels can't be multisampled");
            }
        } else {
            if (pixels == null) {
                glTexImage2D(texType, 0, format, width, height,
                        0, format, GL_UNSIGNED_BYTE, NULL);
            } else {
                glTexImage2D(texType, 0, format,
                        width, height, 0, format,
                        GL_UNSIGNED_BYTE, pixels);
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


}
