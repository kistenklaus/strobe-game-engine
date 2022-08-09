package strobe.graphics.bindables;

import aspect.AspectBindable;
import org.joml.Matrix4f;
import org.joml.Vector3f;
import org.joml.Vector4f;
import strobe.Strobe;
import strobe.StrobeException;

import java.util.HashMap;

import static org.lwjgl.opengl.GL15.*;
import static org.lwjgl.opengl.GL30.glBindBufferBase;
import static org.lwjgl.opengl.GL31.GL_UNIFORM_BUFFER;

/**
 * @version 1.1
 * @author Karl
 */
public class UBO extends AspectBindable {

    private final String name;
    private final int binding_index;
    private int id;
    private final HashMap<String,UboUniform> uniforms = new HashMap<>();
    private final int ubo_byte_size;

    public UBO(String name, int binding_index, String...layout){
        this.name = name;
        this.binding_index = binding_index;

        int ubo_byte_size = 0;
        for(int i=0;i<layout.length;i++){
            String[] split = layout[i].split(" ");
            final String uniform_type = split[0];
            final String uniform_name = split[1];

            Class<?> reference_class;

            int base_alignment;
            int count;

            switch(uniform_type){
                case "mat4":
                    reference_class = Matrix4f.class;
                    base_alignment = 16;
                    count = 4;
                    break;
                case "vec4":
                    reference_class = Vector4f.class;
                    base_alignment = 16;
                    count = 1;
                    break;
                case "vec3":
                    reference_class = Vector3f.class;
                    base_alignment = 16;
                    count = 1;
                    break;
                default:
                    throw new IllegalArgumentException(uniform_type + " is not a defined variable type for a ubo ");
            }
            int aligned_offset = (int) Math.ceil(ubo_byte_size / base_alignment) * base_alignment;

            ubo_byte_size = aligned_offset + base_alignment * count;
            uniforms.put(uniform_name, new UboUniform(reference_class, uniform_name, aligned_offset));
        }
        this.ubo_byte_size = ubo_byte_size;

    }

    @Override
    public void create() {
        id = glGenBuffers();
        glBindBuffer(GL_UNIFORM_BUFFER, id);
        glBufferData(GL_UNIFORM_BUFFER, this.ubo_byte_size, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_UNIFORM_BUFFER, binding_index, id);
    }

    @Override
    public void bind() {

    }

    @Override
    public void unbind() {

    }

    @Override
    public void dispose() {
        glDeleteBuffers(id);
    }

    public void uniformMat4(String uniform_name, Matrix4f mat4){
        UboUniform uniform = uniforms.get(uniform_name);
        if(!uniform.reference_class.equals(Matrix4f.class))
            throw new StrobeException("can't uniform " + uniform_name + " as a mat4");
        float[] data = new float[16];
        mat4.get(data);
        glBindBuffer(GL_UNIFORM_BUFFER, id);
        glBufferSubData(GL_UNIFORM_BUFFER, uniform.offset, data);
    }

    public void uniformVec4(String uniform_name, Vector4f vec4){
        UboUniform uniform = uniforms.get(uniform_name);
        if(!uniform.reference_class.equals(Vector4f.class))
            throw new StrobeException("can't uniform " + uniform_name + " as a vec4");
        float[] data = new float[]{vec4.x, vec4.y, vec4.z, vec4.w};
        glBindBuffer(GL_UNIFORM_BUFFER, id);
        glBufferSubData(GL_UNIFORM_BUFFER, uniform.offset, data);
    }

    public void uniformVec3(String uniform_name, Vector3f vec3){
        UboUniform uniform = uniforms.get(uniform_name);
        if(!uniform.reference_class.equals(Vector3f.class))
            throw new StrobeException("can't uniform " + uniform_name + " as a vec3");
        float[] data = new float[]{vec3.x, vec3.y, vec3.z};
        glBindBuffer(GL_UNIFORM_BUFFER, id);
        glBufferSubData(GL_UNIFORM_BUFFER, uniform.offset, data);
    }

    private class UboUniform{
        private final Class<?> reference_class;
        private final String name;
        private final int offset;
        public UboUniform(Class<?> reference_class, String name, int offset){
            this.reference_class = reference_class;
            this.name = name;
            this.offset = offset;
        }
    }
}
