package org.aspect.graphics.drawable.steps;

import org.aspect.graphics.bindable.Shader;
import org.aspect.graphics.data.Material;
import org.aspect.graphics.drawable.Step;

public abstract class MaterialStep extends Step {

    private static final String DIFFUSE_MAP_NAME = "diffuseMap";
    private static final String SPECULAR_MAP_NAME = "specularMap";

    public MaterialStep(String passName, Shader shader, Material material) {
        super(passName, shader);
        addAttachment(material.getDiffuse().makeAttachment(getShader(), DIFFUSE_MAP_NAME));
        addAttachment(material.getSpecular().makeAttachment(getShader(), SPECULAR_MAP_NAME));
    }
}
