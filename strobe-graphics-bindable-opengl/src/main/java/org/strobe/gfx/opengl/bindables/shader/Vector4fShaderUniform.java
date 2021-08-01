package org.strobe.gfx.opengl.bindables.shader;

import org.joml.Vector4f;
import org.strobe.gfx.Graphics;

import static org.lwjgl.opengl.GL20.glUniform4fv;
import static org.lwjgl.opengl.GL20C.glUniform4f;

class Vector4fShaderUniform extends ShaderUniform<Vector4f>{

    private float x,y,z,w;

    public Vector4fShaderUniform(Shader shader, int location) {
        super(Vector4f.class, shader, location);
    }

    @Override
    public void uniform(Graphics gfx, Vector4f value) {
        if (value.x == x && value.y == y
                &&value.z == z && value.w == w)
            return;
        x = value.x;
        y = value.y;
        z = value.z;
        w = value.w;
        glUniform4f(location, x,y,z,w);
    }

    @Override
    public Vector4f get(Graphics gfx) {
        return new Vector4f(x,y,z,w);
    }
}