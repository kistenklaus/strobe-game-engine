package org.strobe.gfx.lights;

import org.joml.Vector3f;

public abstract class AbstractLight {

    private final Vector3f ambient;
    private final Vector3f diffuse;
    private final Vector3f specular;

    public AbstractLight(Vector3f ambient, Vector3f diffuse, Vector3f specular) {
        this.ambient = new Vector3f(ambient);
        this.diffuse = new Vector3f(diffuse);
        this.specular = new Vector3f(specular);
    }

    public Vector3f getAmbient() {
        return ambient;
    }

    public Vector3f getDiffuse() {
        return diffuse;
    }

    public Vector3f getSpecular() {
        return specular;
    }
}
