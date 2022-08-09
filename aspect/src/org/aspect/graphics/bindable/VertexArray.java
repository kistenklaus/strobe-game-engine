package org.aspect.graphics.bindable;

import org.aspect.datatype.Array;

import static org.lwjgl.opengl.GL11.GL_FLOAT;
import static org.lwjgl.opengl.GL20.*;
import static org.lwjgl.opengl.GL30.*;
import static org.lwjgl.opengl.GL33.glVertexAttribDivisor;

public final class VertexArray extends Bindable {

    private final Array<Attribute> attribs = new Array<>();
    private IndexBuffer ibo = null;

    private int ID;

    private static int hashAcc = 0;
    private final String hash;

    public VertexArray(){
        hash = ""+hashAcc++;
    }

    public void create() {
        ID = glGenVertexArrays();
    }


    public void setAttribute(VertexBuffer vbo, int attributeLocation,
                             int attributeSize, int attributeStride,
                             int attributeCoverage, int attributeDivisor) {

        if(attribs.get(attributeLocation)!=null){
            Attribute attrib = attribs.get(attributeLocation);
            detachVertexBuffer(attrib.getAttributeLocation());
        }
        attribs.set(attributeLocation,
                new Attribute(vbo, attributeLocation, attributeSize, attributeStride,
                        attributeCoverage, attributeDivisor));

        attachVertexBuffer(vbo, attributeLocation, attributeSize, attributeStride,
                attributeCoverage, attributeDivisor);
    }

    private VertexArray attachVertexBuffer(VertexBuffer vbo, int attributeLocation,
                                      int attributeSize, int attributeStride,
                                      int attributeCoverage, int attributeDivisor) {
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
        return this;
    }

    protected VertexArray detachVertexBuffer(int attributeLocation) {
        glBindVertexArray(ID);
        glDisableVertexAttribArray(attributeLocation);
        glBindVertexArray(0);
        return this;
    }

    public VertexArray setIndexBuffer(IndexBuffer ibo){
        if(this.ibo!=null)detachIndexBuffer(this.ibo);
        this.ibo = ibo;
        attachIndexBuffer(ibo);
        return this;
    }

    private void attachIndexBuffer(IndexBuffer ibo) {
        glBindVertexArray(ID);
        ibo.bind();
        glBindVertexArray(0);
    }



    private void detachIndexBuffer(IndexBuffer ibo) {
        glBindVertexArray(ID);
        ibo.unbind();
        glBindVertexArray(0);
    }

    @Override
    public void bind() {
        glBindVertexArray(ID);
    }

    @Override
    public void unbind() {
        glBindVertexArray(0);
    }

    @Override
    public void dispose() {
        glDeleteVertexArrays(ID);
    }

    public IndexBuffer getIbo(){
        return ibo;
    }

    public VertexBuffer getVbo(int location){
        return attribs.get(location).getVbo();
    }

    public String hash(){
        return hash;
    }

    private final class Attribute{
        private final VertexBuffer vbo;
        private final int attributeLocation;
        private final int attributeSize;
        private final int attributeStride;
        private final int attributeCoverage;
        private final int attributeDivisor;
        public Attribute(VertexBuffer vbo, int attributeLocation, int attributeSize,
                         int attributeStride,
                         int attributeCoverage, int attributeDivisor){
            this.vbo = vbo;
            this.attributeLocation = attributeLocation;
            this.attributeSize = attributeSize;
            this.attributeStride = attributeStride;
            this.attributeDivisor = attributeDivisor;
            this.attributeCoverage = attributeCoverage;
        }

        public VertexBuffer getVbo() {
            return vbo;
        }

        public int getAttributeLocation() {
            return attributeLocation;
        }

        public int getAttributeSize() {
            return attributeSize;
        }

        public int getAttributeStride() {
            return attributeStride;
        }

        public int getAttributeCoverage() {
            return attributeCoverage;
        }

        public int getAttributeDivisor() {
            return attributeDivisor;
        }

    }
}
