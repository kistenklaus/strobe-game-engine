package org.strobe.ecs.context.renderer.light;

import org.joml.Vector3f;


public final class DirectionalLight extends LightComponent{

    public DirectionalLight(Vector3f ambient, Vector3f diffuse, Vector3f specular) {
        super(new org.strobe.gfx.lights.DirectionalLight(ambient, diffuse, specular, new Vector3f()));
    }
}
