package org.strobe.gfx.opengl.bindables.shader;

import org.joml.Vector2f;
import org.strobe.gfx.Graphics;

import static org.lwjgl.opengl.GL20.glUniform2f;

public class Vector2fShaderUniform extends ShaderUniform<Vector2f>{

    private float x,y;

    public Vector2fShaderUniform(Shader shader, int location) {
        super(Vector2f.class, shader, location);
    }

    @Override
    public Vector2f get(Graphics gfx) {
        return new Vector2f(x,y);
    }

    @Override
    protected void uniform(Graphics gfx, Vector2f value) {
        if(value.x == x && value.y == y)return;
        x = value.x;
        y = value.y;
        glUniform2f(location, x,y);
    }
}
