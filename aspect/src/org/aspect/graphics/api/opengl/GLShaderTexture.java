package org.aspect.graphics.api.opengl;

import org.aspect.exceptions.AspectGraphicsException;
import org.aspect.graphics.bindable.Shader;
import org.aspect.graphics.bindable.Texture;
import org.aspect.graphics.bindable.shader.ShaderTexture;
import org.aspect.graphics.renderer.Renderer;

import static org.lwjgl.opengl.GL13.GL_TEXTURE0;
import static org.lwjgl.opengl.GL13.glActiveTexture;

public class GLShaderTexture extends ShaderTexture {

    private int sampler;

    public void create(Shader shader, Texture texture, String samplerName) {
        super.create(shader, texture, samplerName);
        if(!(shader instanceof GLShader))
            throw new AspectGraphicsException("a GLShaderTexture doesn't accept a non glShader");
        GLShader glShader = (GLShader) shader;
        sampler = glShader.queryUniform1i(samplerName);
        System.out.println(samplerName+": "+sampler);
    }

    public void bind() {
        glActiveTexture(GL_TEXTURE0 + sampler);
        texture.bind();
    }

    public void prepare(Renderer renderer, Shader shader){
        shader.uniform1i(samplerName, sampler);
    }
}
