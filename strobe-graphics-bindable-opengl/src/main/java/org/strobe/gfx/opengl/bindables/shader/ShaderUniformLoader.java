package org.strobe.gfx.opengl.bindables.shader;

import org.joml.Matrix4f;
import org.joml.Vector2f;
import org.joml.Vector3f;
import org.joml.Vector4f;
import org.strobe.gfx.Graphics;
import org.strobe.gfx.opengl.bindables.texture.Texture2D;

import static org.lwjgl.opengl.GL20.glGetUniformLocation;

public final class ShaderUniformLoader {


    public static ShaderUniform getShaderUniformFor(Graphics gfx, Shader shader, Class<?> type, String name) {
        int loc = glGetUniformLocation(shader.ID, name);
        if (type.equals(Vector3f.class)) {
            return new Vector3fShaderUniform(shader, loc);
        } else if (type.equals(Vector4f.class)) {
            return new Vector4fShaderUniform(shader, loc);
        } else if (type.equals(Texture2D.class)) {
            return new Texture2DShaderUniform(gfx, shader, loc);
        } else if (type.equals(Matrix4f.class)) {
            return new Matrix4fShaderUniform(shader, loc);
        } else if (type.equals(float.class) || type.equals(Float.class)) {
            return new FloatShaderUniform(shader, loc);
        } else if (type.equals(boolean.class) || type.equals(Boolean.class)) {
            return new BooleanShaderUniform(shader, loc);
        } else if (type.equals(Vector2f.class)) {
            return new Vector2fShaderUniform(shader, loc);
        } else {
            throw new IllegalArgumentException("can't uniform type : " + type);
        }
    }


}
