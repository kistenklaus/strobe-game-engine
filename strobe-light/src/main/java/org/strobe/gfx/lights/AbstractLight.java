package org.strobe.gfx.lights;

import org.joml.Vector3f;

public abstract class AbstractLight {

    private final Vector3f ambient;
    private final Vector3f diffuse;
    private final Vector3f specular;
    private final Vector3f position;

    public AbstractLight(Vector3f ambient, Vector3f diffuse, Vector3f specular,
                         Vector3f position) {
        this.ambient = new Vector3f(ambient);
        this.diffuse = new Vector3f(diffuse);
        this.specular = new Vector3f(specular);
        this.position = new Vector3f(position);
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

    public Vector3f getPosition(){
        return position;
    }
}
