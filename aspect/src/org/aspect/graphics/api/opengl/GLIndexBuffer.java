package org.aspect.graphics.api.opengl;

import org.aspect.graphics.bindable.IndexBuffer;

import static org.lwjgl.opengl.GL15.*;

public final class GLIndexBuffer extends IndexBuffer {

    private int ID;
    private int allocated_ints;
    private boolean readonly;

    @Override
    public void create(int[] content, int allocated_ints, boolean readonly) {
        this.allocated_ints = allocated_ints;
        this.readonly = readonly;

        ID = glGenBuffers();
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, allocated_ints * Integer.BYTES,
                readonly?GL_STATIC_DRAW:GL_DYNAMIC_DRAW);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, content);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    @Override
    public void bind() {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
    }

    @Override
    public void unbind() {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    @Override
    public void dispose() {
        glDeleteBuffers(ID);
    }

    public int getSize(){
        return allocated_ints;
    }

    public boolean isReadonly(){
        return readonly;
    }
}
