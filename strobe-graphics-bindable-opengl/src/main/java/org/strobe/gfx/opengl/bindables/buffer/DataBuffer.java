package org.strobe.gfx.opengl.bindables.buffer;

import org.strobe.gfx.Bindable;
import org.strobe.gfx.BindablePool;
import org.strobe.gfx.Graphics;

import java.nio.BufferOverflowException;
import java.nio.ByteBuffer;

import static org.lwjgl.opengl.GL15.*;

public abstract class DataBuffer<T extends BindablePool> extends Bindable<T> {

    protected int ID;
    protected final int target;
    protected int capacity;
    protected final int usage;


    public DataBuffer(Graphics gfx, int target, int usage, boolean createBuffer){
        super(gfx);
        this.target = target;
        this.usage = usage;
        if(createBuffer){
            ID = glGenBuffers();
        }
    }

    public DataBuffer(Graphics gfx, int target, int capacity, int usage) {
        this(gfx, target, usage);
        allocate(gfx, capacity);
    }

    public DataBuffer(Graphics gfx, int target, int usage){
        super(gfx);
        this.target = target;
        this.usage = usage;
        ID = glGenBuffers();
    }

    protected void allocate(Graphics gfx, int capacity){
        this.capacity = capacity;
        gfx.bind(this);
        glBufferData(target, capacity, usage);
        gfx.unbind(this);
    }

    public void reallocate(Graphics gfx){
        allocate(gfx, capacity);
    }

    @Override
    protected void bind(Graphics gfx) {
        glBindBuffer(target, ID);
    }

    @Override
    protected void unbind(Graphics gfx) {
        glBindBuffer(target, 0);
    }

    @Override
    protected void dispose(Graphics gfx) {
        glBindBuffer(target, ID);
        glDeleteBuffers(ID);
    }

    public void bufferSubData(Graphics gfx, int offset, float[] data){
        if(offset + data.length * Float.BYTES > capacity)throw new BufferOverflowException();
        gfx.bind(this);
        if(data.length == data.length*Float.BYTES){
            //assert offset = 0.
            //reallocate memory if possible
            //reallocate(gfx);
        }
        glBufferSubData(target, offset, data);
        gfx.unbind(this);
    }

    public void bufferSubData(Graphics gfx, int offset, int[] data){
        if(offset + data.length * Integer.BYTES > capacity)throw new BufferOverflowException();
        gfx.bind(this);
        glBufferSubData(target, offset, data);
        gfx.unbind(this);
    }

    /**
     * fetches a sub section of the buffer
     * @param gfx the graphics
     * @param offset the offset of the sub section in bytes
     * @param size the size of the sub section in floats
     * @return the sub section of the buffer
     */
    public float[] getBufferSubDataAsFloatArray(Graphics gfx, int offset, int size){
        float[] array = new float[size];
        gfx.bind(this);
        glGetBufferSubData(target, offset, array);
        gfx.unbind(this);
        return array;
    }

    public float[] getBufferAsFloatArray(Graphics gfx){
        return getBufferSubDataAsFloatArray(gfx, 0, capacity/Float.BYTES);
    }

    public int[] getBufferSubDataAsIntegerArray(Graphics gfx, int offset, int size){
        int[] array = new int[size];
        gfx.bind(this);
        glGetBufferSubData(target, offset, array);
        gfx.unbind(this);
        return array;
    }

    public ByteBuffer mapBuffer(Graphics gfx, int operation){
        return glMapBuffer(target, operation);
    }

    public void unmapBuffer(Graphics gfx){
        glUnmapBuffer(target);
    }

    public int[] getBufferAsIntegerArray(Graphics gfx){
        return getBufferSubDataAsIntegerArray(gfx, 0, capacity/Integer.BYTES);
    }

    public int getCapacity(){
        return capacity;
    }

    public int getTarget(){
        return target;
    }

    public final int getID(){
        return ID;
    }
}
