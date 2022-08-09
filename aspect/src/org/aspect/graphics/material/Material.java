package org.aspect.graphics.material;

import org.aspect.Aspect;
import org.aspect.graphics.bindable.Shader;
import org.aspect.graphics.bindable.UniformBuffer;

import java.util.HashMap;

public abstract class Material {

    protected final MaterialAttachment[] attachments;

    private final UniformBuffer sharedUbo;

    public Material(MaterialAttachment... attachments) {
        this.attachments = attachments;
        sharedUbo = null;
    }

    public Material(Shader... attachmentShaders) {
        attachments = new MaterialAttachment[attachmentShaders.length];
        for (int i = 0; i < attachmentShaders.length; i++) {
            attachments[i] = new MaterialAttachment(attachmentShaders[i]);
        }
        sharedUbo = null;
    }

    public Material(Shader[] attachmentShaders, int binding, String... properties) {
        HashMap<String, String> propType = new HashMap<>();
        for (String prop : properties) {
            String[] split = prop.split(" ");
            String type = split[0];
            String name = split[1];
            propType.put(name, type);
        }
        sharedUbo = Aspect.createUniformBuffer(binding, properties);

        attachments = new MaterialAttachment[attachmentShaders.length];
        for (int i = 0; i < attachmentShaders.length; i++) {
            attachments[i] = new MaterialAttachment(attachmentShaders[i], sharedUbo, propType);
        }
    }

    public UniformBuffer getSharedUbo() {
        return sharedUbo;
    }


}
