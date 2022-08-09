package org.aspect.graphics.api.opengl;

import org.aspect.graphics.bindable.IndexBuffer;
import org.aspect.graphics.bindable.VertexArray;
import org.aspect.graphics.bindable.VertexBuffer;

import static org.lwjgl.opengl.GL11.GL_FLOAT;
import static org.lwjgl.opengl.GL20.*;
import static org.lwjgl.opengl.GL30.glBindVertexArray;
import static org.lwjgl.opengl.GL30.glGenVertexArrays;
import static org.lwjgl.opengl.GL33.glVertexAttribDivisor;

public final class GLVertexArray extends VertexArray {

    private int ID;

    private int drawCount;

    @Override
    public void create() {
        ID = glGenVertexArrays();
    }

    @Override
    protected void attachVertexBuffer(VertexBuffer vbo, int attributeLocation, int attributeSize, int attributeStride, int attributeCoverage, int attributeDivisor) {
        glBindVertexArray(ID);
        vbo.bind();

        for (int i = 0; i < attributeCoverage; i++) {
            int pointerLocation = attributeLocation + i;
            int offset = Float.BYTES * i * attributeSize;
            glEnableVertexAttribArray(pointerLocation);
            glVertexAttribPointer(pointerLocation, attributeSize, GL_FLOAT, false, attributeStride * Float.BYTES, offset);
            glVertexAttribDivisor(pointerLocation, attributeDivisor);
        }

        vbo.unbind();
        glBindVertexArray(0);
    }

    @Override
    protected void detachVertexBuffer(VertexBuffer vbo, int attributeLocation,
                                      int attributeSize, int attributeStride,
                                      int attributeCoverage, int attributeDivisor) {
        glBindVertexArray(ID);
        glDisableVertexAttribArray(attributeLocation);
        glBindVertexArray(0);
    }

    @Override
    protected void attachIndexBuffer(IndexBuffer ibo) {
        glBindVertexArray(ID);
        ibo.bind();
        this.drawCount = ibo.getSize();
        glBindVertexArray(0);
    }

    @Override
    protected void detachIndexBuffer(IndexBuffer ibo) {
        glBindVertexArray(ID);
        ibo.unbind();
        glBindVertexArray(0);
    }

    @Override
    public void bind() {
        //System.out.println("vertexArray : bind");
        glBindVertexArray(ID);
    }

    @Override
    public void unbind() {
        glBindVertexArray(0);
    }

    @Override
    public void dispose() {

    }
}
