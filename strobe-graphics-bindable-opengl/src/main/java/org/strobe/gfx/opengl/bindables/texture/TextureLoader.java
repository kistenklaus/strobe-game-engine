package org.strobe.gfx.opengl.bindables.texture;

import org.lwjgl.BufferUtils;
import org.lwjgl.stb.STBImage;
import org.lwjgl.system.MemoryStack;
import org.strobe.gfx.Graphics;
import org.strobe.utils.ResourceLoader;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.IntBuffer;

import static org.lwjgl.stb.STBImage.stbi_image_free;

public final class TextureLoader {


    public static Texture2D loadTexture(Graphics gfx, String resourcePath) {
        PixelBuffer pixels = loadTexturePixelBuffer(resourcePath);
        TextureFormat format;
        if (pixels.getChannels() == 3) {
            format = TextureFormat.RGB_UBYTE;
        } else if (pixels.getChannels() == 4) {
            format = TextureFormat.RGBA_UBYTE;
        } else if (pixels.getChannels() == 1) {
            format = TextureFormat.RED_UBYTE;
        } else {
            throw new RuntimeException();
        }
        Texture2D texture = new Texture2D(gfx, new TextureOptions(format), pixels);
        return texture;
    }

    public static PixelBuffer loadTexturePixelBuffer(String resourcePath) {
        ByteBuffer imageBuffer = null;
        try {
            imageBuffer = ResourceLoader.ioResourceToByteBuffer(resourcePath);
        } catch (IOException e) {
            e.printStackTrace();
        }
        ByteBuffer image;
        int width;
        int height;
        int channels;
        try (MemoryStack stack = MemoryStack.stackPush()) {
            IntBuffer w = stack.mallocInt(1);
            IntBuffer h = stack.mallocInt(1);
            IntBuffer c = stack.mallocInt(1);

            STBImage.stbi_set_flip_vertically_on_load(true);
            image = STBImage.stbi_load_from_memory(imageBuffer, w, h, c, 0);
            if (image == null) {
                throw new RuntimeException("Failed to load image : " + STBImage.stbi_failure_reason());
            }
            width = w.get(0);
            height = h.get(0);
            channels = c.get(0);
        }
        return new PixelBuffer(image, width, height, channels);
    }


    public static final class PixelBuffer {

        private ByteBuffer buffer;
        private final int width;
        private final int height;
        private int channels;

        public PixelBuffer(ByteBuffer buffer, int width, int height, int channels) {
            this.buffer = buffer;
            this.width = width;
            this.height = height;
            this.channels = channels;
        }

        public void convertTo(int desiredChannels) {
            if (channels == desiredChannels) return;
            if (channels == 4 && desiredChannels == 3) {
                ByteBuffer nbuffer = BufferUtils.createByteBuffer(width * height * 3);
                for (int i = 0; i < width * height * 4; i += 4) {
                    float alpha = (buffer.get(i + 3) & 0xFF) / 255.0f;
                    nbuffer.put((byte) ((buffer.get(i) & 0xFF) * alpha));
                    nbuffer.put((byte) ((buffer.get(i + 1) & 0xFF) * alpha));
                    nbuffer.put((byte) ((buffer.get(i + 2) & 0xFF) * alpha));
                }
                nbuffer.flip();
                dispose();
                buffer = nbuffer;
            } else if (channels == 3 && desiredChannels == 4) {
                System.out.println("alive");
                ByteBuffer nbuffer = BufferUtils.createByteBuffer(width * height * 4);

                for (int i = 0; i < width * height * 3; i += 3) {
                    nbuffer.put(buffer.get(i));
                    nbuffer.put(buffer.get(i + 1));
                    nbuffer.put(buffer.get(i + 2));
                    nbuffer.put((byte) 0xFF);
                }
                nbuffer.flip();
                dispose();
                buffer = nbuffer;
            } else {
                throw new UnsupportedOperationException();
            }
            channels = desiredChannels;
        }

        public ByteBuffer getPixelBuffer() {
            return buffer;
        }

        public int getWidth() {
            return width;
        }

        public int getHeight() {
            return height;
        }

        public int getChannels() {
            return channels;
        }

        public void dispose() {
            //maybe not totally disposed!!! because not sure if BufferUtils.create and stbi_load create the same buffers.
            stbi_image_free(buffer);
        }
    }

}
