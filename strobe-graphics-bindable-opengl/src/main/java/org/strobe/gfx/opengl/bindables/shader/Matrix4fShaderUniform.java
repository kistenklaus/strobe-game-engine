package org.strobe.gfx.opengl.bindables.shader;

import org.joml.Matrix4f;
import org.strobe.gfx.Graphics;

import java.nio.FloatBuffer;

import static org.lwjgl.opengl.GL20.glUniformMatrix4fv;

public class Matrix4fShaderUniform extends ShaderUniform<Matrix4f>{

    private final float[] mat4_array = new float[16];

    public Matrix4fShaderUniform(Shader shader, int location) {
        super(Matrix4f.class, shader, location);
    }

    @Override
    public void uniform(Graphics gfx, Matrix4f value) {
        //TODO check if comparing is faster than binding and uploading
        value.get(mat4_array);
        glUniformMatrix4fv(location, false, mat4_array);
    }

    @Override
    public Matrix4f get(Graphics gfx) {
        return new Matrix4f(FloatBuffer.wrap(mat4_array));
    }
}
