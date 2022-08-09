package org.aspect.graphics.data;

import org.aspect.graphics.bindable.Texture;

public class Material {

    private Texture diffuse;
    private Texture specular;

    public Material(Texture diffuse, Texture specular){
        this.diffuse = diffuse;
        this.specular = specular;
    }

    public Texture getDiffuse() {
        return diffuse;
    }

    public Texture getSpecular() {
        return specular;
    }
}
