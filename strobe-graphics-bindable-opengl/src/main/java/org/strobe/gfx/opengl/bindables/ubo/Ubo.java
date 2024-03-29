package org.strobe.gfx.opengl.bindables.ubo;

import org.joml.Matrix4f;
import org.joml.Vector3f;
import org.joml.Vector4f;
import org.strobe.gfx.Graphics;
import org.strobe.gfx.Pool;
import org.strobe.gfx.opengl.bindables.buffer.DataBuffer;
import org.strobe.gfx.opengl.bindables.mapper.Uniform;
import org.strobe.gfx.opengl.bindables.util.TypeUtil;

import java.lang.reflect.Type;
import java.util.HashMap;
import java.util.Map;

import static org.lwjgl.opengl.GL15.*;
import static org.lwjgl.opengl.GL15C.GL_DYNAMIC_DRAW;
import static org.lwjgl.opengl.GL31.GL_UNIFORM_BUFFER;

@Pool(UboPool.class)
public class Ubo extends DataBuffer<UboPool> {

    private final String name;
    private final int bindingIndex;
    private final int[] offsets;
    private final int[] array_size;
    private final int[] alignment;
    private final Type[] types;
    private final Map<String, Integer> nameMap;

    public Ubo(Graphics gfx, String name, int bindingIndex, boolean unique, String... layout) {
        super(gfx, GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW, false);
        this.name = name;
        this.bindingIndex = bindingIndex;

        Ubo pooledUbo = pool.forceAdd(name, bindingIndex, layout);

        if (pooledUbo == null) {
            nameMap = new HashMap<>();
            offsets = new int[layout.length];
            types = new Type[layout.length];
            alignment = new int[layout.length];
            array_size = new int[layout.length];
            int uboByteSize = 0;
            for (int i = 0; i < layout.length; i++) {
                String[] split = layout[i].trim().split(" ");
                if (split.length != 2) throw new IllegalArgumentException("the layout is invalid");
                String varType = split[0];
                String varName = split[1];

                int byteSize;
                if (varType.matches("[a-zA-Z0-9]+\\[[1-9][0-9]*]")) {
                    String subType = varType.substring(0, varType.indexOf("["));
                    array_size[i] = Integer.parseInt(varType.substring(varType.indexOf("[") + 1, varType.indexOf("]")));
                    alignment[i] = Math.max(TypeUtil.getByteAlignmentOfGlslType(subType),16);
                    int typeSize = TypeUtil.getByteSizeOfGlslType(subType);
                    //align element size
                    byteSize = (int) Math.ceil(typeSize / (float) alignment[i]) * alignment[i] * array_size[i];
                    types[i] = TypeUtil.glslTypeToJavaType(subType);
                } else {
                    array_size[i] = -1;
                    alignment[i] = TypeUtil.getByteAlignmentOfGlslType(varType);
                    byteSize = TypeUtil.getByteSizeOfGlslType(varType);
                    types[i] = TypeUtil.glslTypeToJavaType(varType);
                }

                int alignedOffset = (int) (Math.ceil(uboByteSize / (float) alignment[i]) * alignment[i]);
                offsets[i] = alignedOffset;
                nameMap.put(varName, i);
                uboByteSize = alignedOffset + byteSize;
            }
            ID = glGenBuffers();
            capacity = uboByteSize;
            glBindBuffer(target, ID);
            System.out.println("create ubo " + name + " with capacity : " + capacity);
            glBufferData(target, capacity, usage);
            glBindBuffer(target, 0);
            pool.addUbo(this, layout);
        } else {
            offsets = pooledUbo.offsets;
            types = pooledUbo.types;
            capacity = pooledUbo.capacity;
            this.nameMap = pooledUbo.nameMap;
            alignment = pooledUbo.alignment;
            array_size = pooledUbo.array_size;
            if (unique) {
                ID = glGenBuffers();
                glBindBuffer(GL_UNIFORM_BUFFER, ID);
                glBufferData(target, capacity, usage);
                glBindBuffer(GL_UNIFORM_BUFFER, 0);
                pool.forceAdd(this);
            } else {
                ID = pooledUbo.ID;
            }
        }
    }

    public Ubo(Graphics gfx, String name, int bindingIndex, String... layout) {
        this(gfx, name, bindingIndex, false, layout);
    }

    public <T> Uniform<T> getUniform(Class<T> type, String name) {
        Integer index = nameMap.get(name);
        if (index == null) throw new IllegalArgumentException("ubo doesn't have a variable called [" + name + "]");
        if (types[index] != type) throw new IllegalArgumentException(name + " is not of type " + type.getSimpleName());
        if (array_size[index] != -1) {
            //TODO array UboUniforms for directly changing the information of a complete region of the buffer
            throw new UnsupportedOperationException();
        } else {
            return selectUniform(type, offsets[index]);
        }
    }

    @SuppressWarnings("unchecked")
    public <T> Uniform<T>[] getUniformArray(Class<T> baseType, String name) {
        Integer index = nameMap.get(name);
        if (index == null) throw new IllegalArgumentException("ubo doesn't have a variable called [" + name + "]");
        if (types[index] != baseType)
            throw new IllegalArgumentException(name + " is not of type " + baseType.getSimpleName());
        if (array_size[index] == -1)
            throw new IllegalArgumentException(name + " is not a array variable of the uniform");
        Uniform<T>[] uniforms = new Uniform[array_size[index]];
        int offset = offsets[index];
        for (int i = 0; i < uniforms.length; i++) {
            uniforms[i] = selectUniform(types[index], offset);
            offset += Math.ceil(TypeUtil.getByteSizeOfJavaType(types[index])/(float)alignment[index])*alignment[index];
        }
        return uniforms;
    }

    @SuppressWarnings("unchecked")
    private <T> Uniform<T> selectUniform(Type type, int offset) {
        if (type == Matrix4f.class) {
            return (Uniform<T>) new UboUniformMatrix4f(this, offset);
        } else if (type == Vector3f.class) {
            return (Uniform<T>) new UboUniformVector3f(this, offset);
        } else if (type == Integer.class || type == int.class) {
            return (Uniform<T>) new UboUniformInteger(this, offset);
        } else if (type == Vector4f.class) {
            return (Uniform<T>) new UboUniformVector4f(this, offset);
        }
        throw new UnsupportedOperationException();
    }


    public void uniform(Graphics gfx, String uniformName, Matrix4f mat4) {
        Integer index = nameMap.get(uniformName);
        if (index == null)
            throw new IllegalArgumentException("ubo doesn't have a variable called [" + uniformName + "]");
        if (types[index] != Matrix4f.class) throw new IllegalArgumentException(uniformName + " is not of type mat4");
        if (array_size[index] != -1) throw new UnsupportedOperationException();
        float[] mat4_array = new float[16];
        mat4.get(mat4_array);
        bufferSubData(gfx, offsets[index], mat4_array);
    }

    public void uniform(Graphics gfx, String uniformName, Vector3f vec3) {
        Integer index = nameMap.get(uniformName);
        if (index == null)
            throw new IllegalArgumentException("ubo doesn't have a variable called [" + uniformName + "]");
        if (types[index] != Vector3f.class) throw new IllegalArgumentException(uniformName + " is not of type vec3");
        bufferSubData(gfx, offsets[index], new float[]{vec3.x, vec3.y, vec3.z});
    }

    protected void bind(Graphics gfx) {
        super.bind(gfx);
    }

    protected void unbind(Graphics gfx) {
        super.bind(gfx);
    }


    public int getBindingIndex() {
        return bindingIndex;
    }

    public String getName() {
        return name;
    }

    public int[] getOffsets() {
        return offsets;
    }

    public Type[] getTypes() {
        return types;
    }

    @Override
    public String toString() {
        return "Ubo{" +
                "ID=" + ID +
                ", name='" + name + '\'' +
                ", bindingIndex=" + bindingIndex +
                '}';
    }
}
