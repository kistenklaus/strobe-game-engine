package org.aspect.graphics.bindable;

import org.aspect.datatype.Array;

public abstract class VertexArray extends Bindable {

    protected final Array<Attribute> attribs = new Array<>();
    protected IndexBuffer ibo = null;

    private int drawCount;

    public abstract void create();

    public void setIndexBuffer(IndexBuffer ibo) {
        if(this.ibo != null)detachIndexBuffer(ibo);
        this.drawCount = ibo.getSize();
        this.ibo = ibo;
        attachIndexBuffer(ibo);
    }

    public void setAttribute(VertexBuffer vbo, int attributeLocation,
                             int attributeSize, int attributeStride,
                             int attributeCoverage, int attributeDivisor) {

        if(attribs.get(attributeLocation)!=null){
            Attribute attrib = attribs.get(attributeLocation);
            detachVertexBuffer(attrib.getVbo(),
                    attrib.getAttributeLocation(),
                    attrib.getAttributeSize(),
                    attrib.getAttributeStride(),
                    attrib.getAttributeCoverage(),
                    attrib.getAttributeDivisor());
        }
        attribs.set(attributeLocation,
                new Attribute(vbo, attributeLocation, attributeSize, attributeStride,
                        attributeCoverage, attributeDivisor));

        attachVertexBuffer(vbo, attributeLocation, attributeSize, attributeStride,
                attributeCoverage, attributeDivisor);
    }

    protected abstract void attachVertexBuffer(VertexBuffer vbo, int attributeLocation,
                                               int attributeSize, int attributeStride,
                                               int attributeCoverage, int attributeDivisor);

    protected abstract void detachVertexBuffer(VertexBuffer vbo, int attributeLocation,
                                               int attributeSize, int attributeStride,
                                               int attributeCoverage, int attributeDivisor);

    protected abstract void attachIndexBuffer(IndexBuffer ibo);

    protected abstract void detachIndexBuffer(IndexBuffer ibo);

    public void setDrawCount(int drawCount){
        this.drawCount = drawCount;
    }

    public int getDrawCount(){
        return drawCount;
    }

    protected final class Attribute{
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
