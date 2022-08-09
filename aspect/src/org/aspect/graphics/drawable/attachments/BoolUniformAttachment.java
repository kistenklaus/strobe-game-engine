package org.aspect.graphics.drawable.attachments;

import org.aspect.graphics.bindable.Shader;

public class BoolUniformAttachment implements ShaderAttachment {

    private final int uniformLoc;
    private boolean value;

    public BoolUniformAttachment(Shader shader, String uniformName, boolean value){
        uniformLoc = shader.getUniformLocation(uniformName);
    }

    @Override
    public void prepareShader(Shader shader) {
        shader.uniform1b(uniformLoc, value);
    }

    public void set(boolean value){
        this.value = value;
    }

    public boolean get(){
        return value;
    }
}
