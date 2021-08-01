package org.strobe.gfx.opengl.bindables.shader;

import org.strobe.gfx.Graphics;
import org.strobe.gfx.opengl.bindables.texture.Texture2D;

import static org.lwjgl.opengl.GL13.GL_TEXTURE0;
import static org.lwjgl.opengl.GL13.glActiveTexture;
import static org.lwjgl.opengl.GL20.glGetUniformi;
import static org.lwjgl.opengl.GL20.glUniform1i;

public class Texture2DShaderUniform extends ShaderUniform<Texture2D> {

    private static final boolean SAVE_MODE = true;
    private final int sampler;
    private Texture2D value;

    public Texture2DShaderUniform(Graphics gfx, Shader shader, int location) {
        super(Texture2D.class, shader, location);
        gfx.bind(shader);
        synchronized (shader){
            sampler = shader.samplerCount++;
        }
        glUniform1i(location, sampler);
        gfx.unbind(shader);
    }

    @Override
    public void uniform(Graphics gfx, Texture2D value) {
        this.value = value;
    }

    @Override
    public void onShaderBind(Graphics gfx){
        if(sampler != -1 && value != null){
            glActiveTexture(GL_TEXTURE0+sampler);
            gfx.bind(value);
        }
    }

    public void onShaderUnbind(Graphics gfx){
        if(SAVE_MODE){
            if(sampler!=-1 && value != null){
                glActiveTexture(GL_TEXTURE0+sampler);
                gfx.unbind(value);
            }
        }
    }

    @Override
    public Texture2D get(Graphics gfx) {
        return value;
    }
}
