package org.aspect.graphics.bindable;

import org.aspect.exceptions.AspectGraphicsException;
import org.aspect.exceptions.AspectLayoutException;
import org.joml.Matrix4f;
import org.joml.Vector3f;

import java.util.Arrays;
import java.util.HashMap;

import static org.lwjgl.opengl.GL15.*;
import static org.lwjgl.opengl.GL30.glBindBufferBase;
import static org.lwjgl.opengl.GL31.GL_UNIFORM_BUFFER;

public class UniformBuffer extends Bindable {

    private final HashMap<String, Integer> uniformOffsets = new HashMap<>();

    private int ID;
    private final int bindingIndex;
    private final String[] layout;


    private final String hash;

    public UniformBuffer(int bindingIndex, String...layout){
        hash = getClass()+":"+bindingIndex+":"+ Arrays.toString(layout);
        this.layout = layout;
        this.bindingIndex = bindingIndex;
    }

    public void create() {
        int uboByteSize = 0;
        for (int i = 0; i < layout.length; i++) {
            String[] split = layout[i].split(" ");
            if (split.length != 2) throw new AspectLayoutException("illegal ubo layout");
            final String varType = split[0];
            final String varName = split[1];

            int baseAlignment;
            int count;

            switch (varType) {
                case "mat4":
                    baseAlignment = 16;
                    count = 4;
                    break;
                case "vec3":
                    baseAlignment = 16;
                    count = 1;
                    break;
                default:
                    throw new AspectLayoutException(varType + " is not defined");
            }

            int aligned_offset = (int) Math.ceil(uboByteSize / baseAlignment) * baseAlignment;
            uboByteSize = aligned_offset + baseAlignment * count;

            uniformOffsets.put(varName, aligned_offset);
        }

        ID = glGenBuffers();
        glBindBuffer(GL_UNIFORM_BUFFER, ID);
        glBufferData(GL_UNIFORM_BUFFER, uboByteSize, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    @Override
    public void bind() {
        glBindBuffer(GL_UNIFORM_BUFFER, ID);
        glBindBufferBase(GL_UNIFORM_BUFFER, bindingIndex, ID);
    }

    @Override
    public void unbind() {
        glBindBufferBase(GL_UNIFORM_BUFFER, bindingIndex, 0);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    @Override
    public void dispose() {
        glDeleteBuffers(ID);
    }

    public String hash(){
        return hash;
    }

    public void uniformMat4(String uniformName, Matrix4f mat4) {
        Integer offset = uniformOffsets.get(uniformName);
        if (offset == null)
            throw new AspectGraphicsException("can't uniform " + uniformName + " in this uniform block");
        float[] mat4_array = new float[16];
        mat4.get(mat4_array);
        glBindBuffer(GL_UNIFORM_BUFFER, ID);
        glBufferSubData(GL_UNIFORM_BUFFER, offset, mat4_array);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    public void uniformVec3(String uniformName, Vector3f vec3) {
        Integer offset = uniformOffsets.get(uniformName);
        if (offset == null)
            throw new AspectGraphicsException("can't uniform " + uniformName + " in this uniform block");
        float[] vec3_array = new float[]{vec3.x, vec3.y, vec3.z};
        glBindBuffer(GL_UNIFORM_BUFFER, ID);
        glBufferSubData(GL_UNIFORM_BUFFER, offset, vec3_array);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }


}
