package org.strobe.utils;

import org.lwjgl.BufferUtils;

import java.io.*;
import java.net.URL;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.charset.StandardCharsets;

public final class ResourceLoader {

    public static String readContent(String resourcePath) {
        InputStream in = ResourceLoader.class.getClassLoader().getResourceAsStream(resourcePath);
        if(in == null) try {
            throw new FileNotFoundException(resourcePath);
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        }
        StringBuilder str = new StringBuilder();
        try {
            InputStreamReader streamReader = new InputStreamReader(in, StandardCharsets.UTF_8);
            BufferedReader br = new BufferedReader(streamReader);
            String line;
            while ((line = br.readLine()) != null) {
                str.append(line).append("\n");
            }
            if (str.length() != 0)
                str.delete(str.length() - "\n".length(), str.length());
        } catch (IOException e) {
            e.printStackTrace();
        }
        return str.toString();
    }


    public static ByteBuffer ioResourceToByteBuffer(String resource) throws IOException {
        return ioResourceToByteBuffer(resource, 8 * 1024);
    }

    public static ByteBuffer ioResourceToByteBuffer(String resource, int bufferSize) throws IOException {
        ByteBuffer buffer;
        URL url = Thread.currentThread().getContextClassLoader().getResource(resource);
        if (url == null)
            throw new IOException("Classpath resource not found: " + resource);
        File file = new File(url.getFile());
        if (file.isFile()) {
            FileInputStream fis = new FileInputStream(file);
            FileChannel fc = fis.getChannel();
            buffer = fc.map(FileChannel.MapMode.READ_ONLY, 0, fc.size());
            fc.close();
            fis.close();
        } else {
            buffer = BufferUtils.createByteBuffer(bufferSize);
            InputStream source = url.openStream();
            if (source == null)
                throw new FileNotFoundException(resource);
            try {
                byte[] buf = new byte[8192];
                while (true) {
                    int bytes = source.read(buf, 0, buf.length);
                    if (bytes == -1)
                        break;
                    if (buffer.remaining() < bytes)
                        buffer = resizeBuffer(buffer, Math.max(buffer.capacity() * 2, buffer.capacity() - buffer.remaining() + bytes));
                    buffer.put(buf, 0, bytes);
                }
                buffer.flip();
            } finally {
                source.close();
            }
        }
        return buffer;
    }

    private static ByteBuffer resizeBuffer(ByteBuffer buffer, int newCapacity) {
        ByteBuffer newBuffer = BufferUtils.createByteBuffer(newCapacity);
        buffer.flip();
        newBuffer.put(buffer);
        return newBuffer;
    }
}