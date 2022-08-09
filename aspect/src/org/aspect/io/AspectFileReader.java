package org.aspect.io;

import org.joml.Vector3i;
import org.lwjgl.BufferUtils;

import java.io.*;
import java.nio.ByteBuffer;
import java.nio.IntBuffer;
import java.nio.file.Files;
import java.nio.file.Paths;

import static org.lwjgl.stb.STBImage.*;

public class AspectFileReader {
    private static AspectFileReader instance;

    private AspectFileReader() {
    }

    private static AspectFileReader getInstance() {
        if (instance == null) instance = new AspectFileReader();
        return instance;
    }

    public static final String readFile(String path) {
        try {
            return new String(Files.readAllBytes(Paths.get(path)));
        } catch (IOException e) {
            e.printStackTrace();
        }
        return null;
    }

    public static final String readFileFromClasspath(String classpath) {
        BufferedReader br = null;

        StringBuilder content = new StringBuilder();
        InputStream in = getInstance().getClass().getResourceAsStream(classpath);
        try{
            br = new BufferedReader(new InputStreamReader(in));
        }catch(NullPointerException e){
            try {
                throw new FileNotFoundException(classpath);
            } catch (FileNotFoundException fileNotFoundException) {
                fileNotFoundException.printStackTrace();
            }
        }
        try {
            String line;
            while ((line = br.readLine()) != null)
                content.append(line).append("\n");

            in.close();
            br.close();
        } catch (IOException e) {
            e.printStackTrace();
        }

        return content.toString();
    }

    public static ByteBuffer loadImage(String FILEPATH, Vector3i specs) {
        IntBuffer width = BufferUtils.createIntBuffer(1);
        IntBuffer height = BufferUtils.createIntBuffer(1);
        IntBuffer channels = BufferUtils.createIntBuffer(1);
        if (!new File(FILEPATH).exists())
            try {
                throw new FileNotFoundException(FILEPATH);
            } catch (FileNotFoundException e) {
                e.printStackTrace();
            }
        stbi_set_flip_vertically_on_load(true);
        ByteBuffer pixels = stbi_load(FILEPATH, width, height, channels, 0);
        specs.set(width.get(0), height.get(0), channels.get(0));
        return pixels;
    }

    public static void freeImage(ByteBuffer pixels) {
        stbi_image_free(pixels);
    }
}
