package org.strobe.gfx.opengl.bindables.shader;

import org.joml.Vector3f;
import org.strobe.gfx.Graphics;

import static org.lwjgl.opengl.GL20.glUniform3f;
import static org.lwjgl.opengl.GL20.glUniform3fv;

class Vector3fShaderUniform extends ShaderUniform<Vector3f> {

    private float x,y,z;

    protected Vector3fShaderUniform(Shader shader, int location) {
        super(Vector3f.class, shader, location);
    }

    @Override
    public void uniform(Graphics gfx, Vector3f value) {
        if(value.x == x && value.y == y
                && value.z == z)return;
        x = value.x;
        y = value.y;
        z = value.z;
        glUniform3f(location, x,y,z);
    }

    @Override
    public Vector3f get(Graphics gfx) {
        return new Vector3f(x,y,z);
    }
}