package org.aspect.graphics.api.opengl;

import org.aspect.graphics.bindable.VertexBuffer;

import static org.lwjgl.opengl.GL15.*;

public final class GLVertexBuffer extends VertexBuffer {

    private int ID;
    private int allocated_size;
    private boolean readonly;

    @Override
    public void create(float[] content, int allocated_floats, boolean readonly) {

        this.allocated_size = allocated_floats;
        this.readonly = readonly;

        ID = glGenBuffers();

        glBindBuffer(GL_ARRAY_BUFFER, ID);
        glBufferData(GL_ARRAY_BUFFER, allocated_floats * Float.BYTES,
                readonly ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, content);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    @Override
    public void bind() {
        glBindBuffer(GL_ARRAY_BUFFER, ID);
    }

    @Override
    public void unbind() {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    @Override
    public void dispose() {
        unbind();
        glDeleteBuffers(ID);
    }

    public int getSize(){
        return allocated_size;
    }

    public boolean isReadonly(){
        return readonly;
    }
}
