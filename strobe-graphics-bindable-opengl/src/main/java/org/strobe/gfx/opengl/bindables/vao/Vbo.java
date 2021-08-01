package org.strobe.gfx.opengl.bindables.vao;

import org.strobe.gfx.Graphics;
import org.strobe.gfx.Pool;
import org.strobe.gfx.opengl.bindables.buffer.DataBuffer;
import org.strobe.gfx.opengl.bindables.util.TypeUtil;

import java.nio.ByteBuffer;

import static org.lwjgl.opengl.GL15.*;
import static org.lwjgl.opengl.GL20.*;
import static org.lwjgl.opengl.GL33.glVertexAttribDivisor;


@Pool(VboPool.class)
public class Vbo extends DataBuffer<VboPool> {

    private static int calculateSize(int[] strides, int[] types) {
        int size = 0;
        for (int i = 0; i < strides.length; i++) {
            size += strides[i] * TypeUtil.getSizeOfGlTypeEnum(types[i]);
        }
        return size;
    }

    private static int[] defaultTypes(int attribCount) {
        int[] types = new int[attribCount];
        for (int i = 0; i < types.length; i++) types[i] = GL_FLOAT;
        return types;
    }

    private int[] locations;
    private int[] strides;
    private int[] types;        //glEnum
    private int[] divisors;
    private int[] coverages;
    private int[] offsets;
    private int vertexSize;

    public Vbo(Graphics gfx, int vertexCount, int[] locations, int[] strides, int[] divisors, int[] coverages, int usage) {
        this(gfx, vertexCount, locations, strides, defaultTypes(locations.length), divisors, coverages, usage);
    }

    public Vbo(Graphics gfx, int vertexCount, int[] locations, int[] strides, int[] types, int[] divisors, int[] coverages, int usage) {
        super(gfx, GL_ARRAY_BUFFER, calculateSize(strides, types) * vertexCount, usage);
        pool.add(this);
        //check if valid arguments
        int attribCount = locations.length;
        if (strides.length != attribCount) throw new IllegalArgumentException("the length of strides is invalid");
        if (types.length != attribCount) throw new IllegalArgumentException("the length of types is invalid");
        if (divisors.length != attribCount) throw new IllegalArgumentException("the length of divisors is invalid");
        this.locations = locations;
        this.strides = strides;
        this.types = types;
        this.divisors = divisors;
        this.coverages = coverages;
        this.vertexSize = capacity / vertexCount;

        //calculate offsets.
        offsets = new int[attribCount];
        int offset = 0;
        for (int i = 0; i < attribCount; i++) {
            offsets[i] = offset;
            offset += strides[i] * coverages[i] * TypeUtil.getSizeOfGlTypeEnum(types[i]);
        }
    }

    protected void connectToBoundVao(Graphics gfx) {
        gfx.bind(this);

        for (int i = 0; i < locations.length; i++) {
            for (int j = 0; j < coverages[i]; j++) {
                int location = locations[i] + j;
                glEnableVertexAttribArray(location);
                glVertexAttribPointer(location, strides[i], types[i], false, vertexSize, offsets[i] + j * strides[i] * TypeUtil.getSizeOfGlTypeEnum(types[i]));
                glVertexAttribDivisor(location, divisors[i]);
                glDisableVertexAttribArray(location);
            }
        }

        gfx.unbind(this);
    }

    public void bufferLocation(Graphics gfx, int location, float[] data) {
        int j = -1;
        for (int i = 0; i < locations.length; i++) {
            if (location == locations[i]) {
                j = i;
                break;
            }
        }
        if (j == -1) throw new IllegalArgumentException("the vbo doesn't have an attribute of location: " + location);
        if (types[j] != GL_FLOAT)
            throw new IllegalArgumentException("a non GL_FLOAT attribute can't be buffered using this function");

        if (locations.length == 1) {
            bufferSubData(gfx, 0, data);
            return;
        }
        int strideInBytes = strides[j] * Float.BYTES;
        int offset = offsets[j];
        gfx.bind(this);
        ByteBuffer byteBuffer = glMapBuffer(target, GL_WRITE_ONLY);
        byteBuffer.position(offset);
        for (int i = 0; i < data.length; i++) {
            byteBuffer.putFloat(data[i]);
            if ((byteBuffer.position() - offset) % vertexSize >= strideInBytes && i != data.length - 1)
                byteBuffer.position(byteBuffer.position() + (vertexSize - strideInBytes));
        }
        byteBuffer.flip();
        glUnmapBuffer(target);
        gfx.unbind(this);
    }

    public void bufferLocation(Graphics gfx, int location, int[] data) {
        int j = -1;
        for (int i = 0; i < locations.length; i++) {
            if (location == locations[i]) {
                j = i;
                break;
            }
        }
        if (j == -1) throw new IllegalArgumentException("the vbo doesn't have an attribute of location: " + location);
        if (types[j] != GL_INT || types[j] == GL_UNSIGNED_INT)
            throw new IllegalArgumentException("a non GL_FLOAT attribute can't be buffered using this function");

        if (locations.length == 1) {
            bufferSubData(gfx, 0, data);
            return;
        }

        int strideInBytes = strides[j] * Integer.BYTES;

        int offset = offsets[j];
        gfx.bind(this);
        ByteBuffer byteBuffer = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
        byteBuffer.position(offset);
        for (int i = 0; i < data.length; i++) {
            byteBuffer.putInt(data[i]);
            if ((byteBuffer.position() - offset) % vertexSize >= strideInBytes && i != data.length - 1)
                byteBuffer.position(byteBuffer.position() + (vertexSize - strideInBytes));
        }
        byteBuffer.flip();
        glUnmapBuffer(GL_ARRAY_BUFFER);
        gfx.unbind(this);
    }

    public int[] getLocations() {
        return locations;
    }

    public int[] getStrides() {
        return strides;
    }

    public int[] getTypes() {
        return types;
    }

    public int[] getDivisors() {
        return divisors;
    }

    public int[] getCoverages() {
        return coverages;
    }

    public int[] getOffsets() {
        return offsets;
    }

    public int getVertexSize() {
        return vertexSize;
    }
}
