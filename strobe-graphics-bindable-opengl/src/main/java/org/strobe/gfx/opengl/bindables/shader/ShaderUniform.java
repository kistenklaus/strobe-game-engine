package org.strobe.gfx.opengl.bindables.shader;

import org.strobe.gfx.Graphics;
import org.strobe.gfx.opengl.bindables.mapper.Uniform;

public abstract class ShaderUniform<T> implements Uniform<T> {

    protected final Shader shader;
    protected final int location;
    protected final Class<T> type;

    public ShaderUniform(Class<T> type, Shader shader, int location){
        this.type = type;
        this.shader = shader;
        this.location = location;
    }

    protected void onShaderBind(Graphics gfx){}

    protected void onShaderUnbind(Graphics gfx){}

    public Shader getShader(){
        return shader;
    }

    public final void set(Graphics gfx, T value){
        boolean bound = shader.isBound();
        if(!bound)gfx.bind(shader);
        uniform(gfx, value);
        if(!bound)gfx.unbind(shader);
    }

    protected abstract void uniform(Graphics gfx, T value);

    public Class<T> getType(){
        return type;
    }

}
