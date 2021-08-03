package org.strobe.gfx.opengl.bindables.shader;

import org.joml.Matrix3f;
import org.strobe.gfx.Graphics;

import java.nio.FloatBuffer;

import static org.lwjgl.opengl.GL20.glUniform3fv;

public final class Matrix3fShaderUniform extends ShaderUniform<Matrix3f>{

    private final float[] mat3_array = new float[9];

    public Matrix3fShaderUniform(Shader shader, int location) {
        super(Matrix3f.class, shader, location);
    }

    @Override
    public Matrix3f get(Graphics gfx) {
        return new Matrix3f(FloatBuffer.wrap(mat3_array));
    }

    @Override
    protected void uniform(Graphics gfx, Matrix3f value) {
        value.get(mat3_array);
        glUniform3fv(location, mat3_array);
    }
}
