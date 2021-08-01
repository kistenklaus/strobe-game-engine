package org.strobe.gfx.opengl.bindables.shader;

import org.strobe.gfx.Graphics;

import static org.lwjgl.opengl.GL20.glUniform1i;

public class BooleanShaderUniform extends ShaderUniform<Boolean>{

    private Boolean value = null;

    public BooleanShaderUniform(Shader shader, int location) {
        super(Boolean.class, shader, location);
    }

    @Override
    public void uniform(Graphics gfx, Boolean value) {
        if(value == null)throw new IllegalArgumentException("can't uniform null");
        if(value.equals(this.value))return;
        this.value = value;
        glUniform1i(location, value?1:0);
    }

    @Override
    public Boolean get(Graphics gfx) {
        if(value == null)throw new RuntimeException("boolean uniform value is undefined");
        return value;
    }
}
