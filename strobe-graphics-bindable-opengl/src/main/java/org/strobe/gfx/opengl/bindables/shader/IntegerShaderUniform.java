package org.strobe.gfx.opengl.bindables.shader;

import org.strobe.gfx.Graphics;

import static org.lwjgl.opengl.GL20.glUniform1i;

public final class IntegerShaderUniform extends ShaderUniform<Integer>{

    private Integer value = null;

    public IntegerShaderUniform(Shader shader, int location) {
        super(int.class, shader, location);
    }

    @Override
    protected void uniform(Graphics gfx, Integer value) {
        if(value.equals(this.value))return;
        this.value = value;
        glUniform1i(location, value);
    }

    @Override
    public Integer get(Graphics gfx) {
        return this.value;
    }

}
