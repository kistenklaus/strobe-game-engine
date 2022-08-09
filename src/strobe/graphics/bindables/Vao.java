package strobe.graphics.bindables;

import aspect.AspectBindable;
import strobe.StrobeException;
import strobe.datatypes.Array;

import static org.lwjgl.opengl.GL11.GL_FLOAT;
import static org.lwjgl.opengl.GL20.glEnableVertexAttribArray;
import static org.lwjgl.opengl.GL20.glVertexAttribPointer;
import static org.lwjgl.opengl.GL30.*;
import static org.lwjgl.opengl.GL33.glVertexAttribDivisor;

public class Vao extends AspectBindable {

    private final Array<Attribute> attribs = new Array<>(4);
    private int id;
    private Ebo ebo;
    private boolean finalized = false;

    public Vao() {

    }

    @Override
    public void create() {
        //error checking:
        if (finalized) throw new StrobeException("can't create the vao : vao was already created");
        finalized = true;

        if (ebo == null) throw new StrobeException("can't create the vao: no ebo defined");
        if (attribs.size() <= 0) throw new StrobeException("can't create the vao : no vbos defined");


        int vertex_count = -1;
        for (Attribute attrib : attribs) {
            int vbo_size = attrib.vbo.getSize();
            int attrib_vertex_count = vbo_size / attrib.size;
            if (vertex_count == -1) vertex_count = attrib_vertex_count;
            else if (vertex_count != attrib_vertex_count)
                throw new StrobeException("can't create the vao : all attributes must contain the same amount of vertices");
        }
        //creating the vbos
        for (Attribute attrib : attribs) {
            attrib.vbo.intern_create();
        }
        //creating the vao
        id = glGenVertexArrays();
        glBindVertexArray(id);
        //binding the vbos to the vao
        for (Attribute attrib : attribs) {
            attrib.vbo.bind();
            for (int i = 0; i < attrib.coverage; i++) {
                int pointer_location = attrib.location + i;
                int offset = Float.BYTES * i * attrib.size;
                glEnableVertexAttribArray(pointer_location);
                glVertexAttribPointer(pointer_location, attrib.size, GL_FLOAT, false, attrib.stride * Float.BYTES, offset);
                glVertexAttribDivisor(pointer_location, attrib.divisor);
            }
            attrib.vbo.unbind();
        }
        //creating the ebo
        ebo.intern_create();
        //binding the ebo to the vao
        ebo.bind();

        glBindVertexArray(0);
    }

    /**
     * @param attribute_loc
     * @param attribute_size
     * @param vbo
     * @return
     */
    public Vao setAttribute(int attribute_loc, int attribute_size, FloatVbo vbo) {
        return setAttribute(attribute_loc, attribute_size, attribute_size, 1, 0, vbo);
    }

    /**
     * @param attribute_loc
     * @param attribute_size
     * @param attribute_divisor
     * @param vbo
     * @return
     */
    public Vao setAttribute(int attribute_loc, int attribute_size, int attribute_divisor, FloatVbo vbo) {
        return setAttribute(attribute_loc, attribute_size, attribute_size, 1, attribute_divisor, vbo);
    }

    /**
     * @param attribute_loc
     * @param attribute_size
     * @param attribute_stride
     * @param attribute_coverage
     * @param attribute_divisor
     * @param vbo
     * @return
     */
    public Vao setAttribute(int attribute_loc, int attribute_size, int attribute_stride, int attribute_coverage, int attribute_divisor, FloatVbo vbo) {
        if (finalized) throw new StrobeException("can't set Attribute : the vao is finalized");
        if (vbo == null) throw new NullPointerException("can't set Attribute : vbo is null");
        attribs.add(new Attribute(attribute_loc, attribute_size, attribute_stride, attribute_coverage, attribute_divisor, vbo));
        return this;
    }

    public void updateAttribute(int location, int from, int to, float[] data) {
        Attribute attrib = attribs.get(location);
        FloatVbo vbo = attrib.vbo;
        if ((to - from) * attrib.size == data.length) vbo.buffer(from * attrib.size, data);
        else {
            float[] sub_data = new float[(to - from) * attrib.size];
            for (int i = 0; i < sub_data.length; i++) sub_data[i] = data[i];
            vbo.buffer(from * attrib.size, sub_data);
        }
    }

    /**
     * @param ebo
     * @return
     */
    public Vao setEbo(Ebo ebo) {
        this.ebo = ebo;
        return this;
    }

    @Override
    public void bind() {
        glBindVertexArray(id);
        for (Attribute attrib : attribs) attrib.vbo.dobuffer();
        ebo.dobuffer();
    }

    @Override
    public void unbind() {
        glBindVertexArray(id);
    }

    @Override
    public void dispose() {
        for (Attribute attrib : attribs) attrib.vbo.dispose();
        glDeleteVertexArrays(id);
    }


    private class Attribute {
        private final FloatVbo vbo;
        private final int location;
        private final int size;
        private final int stride;
        private final int coverage;
        private final int divisor;

        private Attribute(int attribute_loc, int attribute_size, int attribute_stride, int attribute_coverage, int attribute_divisor, FloatVbo vbo) {
            location = attribute_loc;
            size = attribute_size;
            stride = attribute_stride;
            coverage = attribute_coverage;
            divisor = attribute_divisor;
            this.vbo = vbo;
        }
    }
}
