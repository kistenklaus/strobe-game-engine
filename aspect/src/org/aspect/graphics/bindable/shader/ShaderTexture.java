package org.aspect.graphics.bindable.shader;

import org.aspect.graphics.bindable.Shader;
import org.aspect.graphics.bindable.Texture;
import org.aspect.graphics.renderer.Renderer;

public class ShaderTexture extends ShaderBindable{

    protected Texture texture;
    protected String samplerName;

    public void create(Shader shader, Texture texture, String samplerName){
        this.texture = texture;
        this.samplerName = samplerName;
    }

    @Override
    public void bind() {
        texture.bind();
    }

    @Override
    public void prepare(Renderer renderer, Shader shader) {
    }

    @Override
    public void unbind() {
        texture.unbind();
    }

    @Override
    public void dispose() {
        texture.dispose();
    }
}
