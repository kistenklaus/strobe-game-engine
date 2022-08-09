package strobe.components;

import marshal.MarshalComponent;
import strobe.StrobeException;
import strobe.datatypes.Array;
import strobe.graphics.bindables.Ebo;
import strobe.graphics.bindables.FloatVbo;
import strobe.graphics.bindables.Vao;

public class Mesh extends MarshalComponent {

    private Vao vao;
    private boolean finalized = false;

    private Array<Attribute> attributes = new Array<>();

    private int[] indices;
    private int allocated_indices;
    private int indices_usage;

    public Mesh() {

    }

    public Vao createVao() {
        finalized = true;
        vao = new Vao();
        for (Attribute attrib : attributes) {
            FloatVbo vbo = new FloatVbo(attrib.content, attrib.allocated_floats, attrib.usage);
            vao.setAttribute(attrib.location, attrib.size, attrib.stride, attrib.coverage, attrib.divisor, vbo);
        }
        Ebo ebo = new Ebo(indices, allocated_indices, indices_usage);
        vao.setEbo(ebo);
        return vao;
    }

    /**
     * @param location
     * @param allocated_size
     * @param stride
     * @param size
     * @param divisor
     * @param coverage
     * @param content
     * @param usage
     */
    public synchronized void setAttribute(int location, int allocated_size, int stride, int size, int divisor, int coverage, float[] content, int usage) {
        if (finalized) throw new StrobeException("can't create an new Attribute on an finalized mesh");
        attributes.set(location, new Attribute(location, allocated_size, stride, size, divisor, coverage, content, usage));
    }

    public synchronized void updateAttribute(int location, int from, int to, float[] data) {
        Attribute attrib = attributes.get(location);
        if (attrib == null) throw new StrobeException("can't update an attribute that was not created");
        if (from > to || to > attrib.allocated_floats || from < 0)
            throw new IndexOutOfBoundsException("invalid : [from-to]");
        if (!finalized) {
            float[] content = attrib.content;
            for (int i = 0; i < to - from; i++) {
                content[from + i] = data[i];
            }
        } else {
            vao.updateAttribute(location, from, to, data);
        }
    }

    public synchronized void setIndices(int allocated_indices, int[] indices, int indices_usage) {
        if (finalized) throw new StrobeException("can't initialize indices on an finalized mesh");
        this.indices = indices;
        this.indices_usage = indices_usage;
        this.allocated_indices = allocated_indices;
    }

    public synchronized void updateIndices(int from, int to, int[] data) {

    }

    private class Attribute {
        private final int location;
        private final int allocated_floats;
        private final float[] content;
        private final int stride;
        private final int size;
        private final int divisor;
        private final int coverage;
        private final int usage;

        public Attribute(int location, int allocated_floats, int stride, int size, int divisor, int coverage, float[] content, int usage) {
            this.location = location;
            this.allocated_floats = allocated_floats;
            this.stride = stride;
            this.size = size;
            this.content = content;
            this.divisor = divisor;
            this.coverage = coverage;
            this.usage = usage;
        }
    }

}
