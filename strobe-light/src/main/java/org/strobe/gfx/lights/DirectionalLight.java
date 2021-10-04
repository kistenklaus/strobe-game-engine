package org.strobe.gfx.lights;

import org.joml.Vector3f;

public final class DirectionalLight extends AbstractLight{

    public DirectionalLight(Vector3f ambient, Vector3f diffuse, Vector3f specular, Vector3f position) {
        super(ambient, diffuse, specular, position);
    }

    public Vector3f getDirection() {
        return getPosition();
    }


}
