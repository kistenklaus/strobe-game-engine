package org.aspect.graphics.drawable.attachments;

import org.aspect.exceptions.AspectInvalidUniform;
import org.aspect.graphics.bindable.Shader;
import org.aspect.graphics.bindable.Texture;

import static org.lwjgl.opengl.GL13.GL_TEXTURE0;
import static org.lwjgl.opengl.GL13.glActiveTexture;

public class TextureAttachment extends BindableAttachment<Texture>
        implements ShaderAttachment {

    private final int samplerBinding;
    private final String samplerName;

    public TextureAttachment(Texture texture, Shader shader, String samplerName) {
        super(texture);
        this.samplerName = samplerName;
        int temp;
        try{
            temp = shader.queryUniform1i(samplerName);
        }catch(AspectInvalidUniform e){
            temp = -1;
        }
        samplerBinding = temp;

    }

    public void bind(){
        if(samplerBinding != -1){
            glActiveTexture(GL_TEXTURE0+samplerBinding);
            getBindable().bind();
        }
    }

    @Override
    public void prepareShader(Shader shader) {
        shader.uniform1i(samplerName,samplerBinding);
    }
}
