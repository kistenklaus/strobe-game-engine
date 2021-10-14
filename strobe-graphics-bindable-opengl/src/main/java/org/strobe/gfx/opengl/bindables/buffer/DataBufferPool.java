package org.strobe.gfx.opengl.bindables.buffer;

import org.strobe.debug.Debug;
import org.strobe.debug.Debuggable;
import org.strobe.gfx.BindablePool;
import org.strobe.gfx.BindingMethod;
import org.strobe.gfx.Graphics;

import java.util.ArrayList;
import java.util.Iterator;

public abstract class DataBufferPool<T extends DataBuffer> extends BindablePool<T> implements Debuggable {

    @Debug(inherit=true)
    private final ArrayList<T> buffers = new ArrayList<>();

    public DataBufferPool(BindingMethod method) {
        super(method);
    }

    public void add(T buffer){
        buffers.add(buffer);
    }

    @Override
    public void dispose(Graphics gfx, T buffer){
        buffers.remove(buffer);
        buffer.dispose(gfx);
    }

    @Override
    public Iterator<T> iterator() {
        return buffers.iterator();
    }

    public Iterable<T> buffers(){
        return buffers;
    }
}
