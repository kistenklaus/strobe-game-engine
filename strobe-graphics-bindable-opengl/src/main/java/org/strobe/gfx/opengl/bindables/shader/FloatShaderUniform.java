package org.strobe.gfx.opengl.bindables.shader;

import org.strobe.gfx.Graphics;

import static org.lwjgl.opengl.GL20.glUniform1f;

public class FloatShaderUniform extends ShaderUniform<Float>{

    private float value;

    public FloatShaderUniform(Shader shader, int location) {
        super(Float.class, shader, location);
    }

    @Override
    public void uniform(Graphics gfx, Float value) {
        if(Math.abs(this.value - value) < 0.001f)return;
        this.value = value;
        glUniform1f(location, this.value);
    }

    @Override
    public Float get(Graphics gfx) {
        return value;
    }
}
