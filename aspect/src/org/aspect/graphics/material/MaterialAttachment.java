package org.aspect.graphics.material;

import org.aspect.Aspect;
import org.aspect.exceptions.AspectInvalidMaterialPropertyException;
import org.aspect.graphics.bindable.Shader;
import org.aspect.graphics.bindable.Texture;
import org.aspect.graphics.bindable.UniformBuffer;
import org.aspect.graphics.drawable.Attachment;
import org.aspect.graphics.drawable.attachments.TextureAttachment;
import org.aspect.graphics.renderer.CullFace;
import org.aspect.graphics.renderer.Renderer;
import org.joml.Vector3f;
import org.joml.Vector4f;

import java.util.ArrayList;
import java.util.HashMap;

public class MaterialAttachment implements Attachment {

    private UniformBuffer matUbo;

    private Shader shader;

    private HashMap<String, String> propType;
    private ArrayList<TextureAttachment> textures = new ArrayList<>();

    private CullFace cullFace = CullFace.BACK;

    public MaterialAttachment(Shader shader, int bindingIndex, String... properties) {
        this.shader = shader;
        this.propType = new HashMap<>();
        if (properties.length != 0) {
            for (String property : properties) {
                String[] split = property.split(" ");
                String type = split[0];
                String name = split[1];
                propType.put(name, type);
            }
            matUbo = Aspect.createUniformBuffer(bindingIndex, properties);
        }
    }

    public MaterialAttachment(Shader shader, UniformBuffer uniformBuffer, HashMap<String, String> propType) {
        this.shader = shader;
        matUbo = uniformBuffer;
        this.propType = propType;

    }

    public MaterialAttachment(Shader shader) {
        this.shader = shader;
    }

    public void setPropertyVec3(String name, Vector3f vec3) {
        checkType(name, "vec3");
        matUbo.uniformVec3(name, vec3);
    }

    public void setPropertyVec4(String name, Vector4f vec4) {
        checkType(name, "vec4");
        matUbo.uniformVec4(name, vec4);
    }

    public void setPropertyFloat(String name, float value) {
        checkType(name, "float");
        matUbo.uniform1f(name, value);
    }

    private void checkType(String name, String type) {
        String propertyType = propType.get(name);
        if (propertyType == null) throw new AspectInvalidMaterialPropertyException(name + " is not a property");
        if (!propertyType.equals(type))
            throw new AspectInvalidMaterialPropertyException(name + " is not a " + type);
    }

    public void addTexture(String sampler, Texture texture) {
        TextureAttachment tex = texture.makeAttachment(shader, sampler);
        textures.add(tex);
    }

    public void attach(Shader shader, Renderer renderer) {
        if (matUbo != null) matUbo.bind();
        for (TextureAttachment tex : textures) {
            tex.attach(shader, renderer);
        }
    }

    public Shader getShader() {
        return shader;
    }

    public CullFace getCullFace() {
        return cullFace;
    }

    public void setCullFace(CullFace cullFace) {
        this.cullFace = cullFace;
    }

}
