package org.strobe.gfx.lights;

import org.joml.Vector3f;

public final class DirectionalLight extends AbstractLight{

    private final Vector3f direction;

    public DirectionalLight(Vector3f ambient, Vector3f diffuse, Vector3f specular, Vector3f direction) {
        super(ambient, diffuse, specular);
        this.direction = new Vector3f(direction);
    }

    public Vector3f getDirection() {
        return direction;
    }
}
