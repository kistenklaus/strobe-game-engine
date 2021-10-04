package org.strobe.ecs.context.renderer.light;

import org.joml.Vector3f;


public final class DirectionalLight extends LightComponent{

    public DirectionalLight(Vector3f ambient, Vector3f diffuse, Vector3f specular) {
        super(new org.strobe.gfx.lights.DirectionalLight(ambient, diffuse, specular, new Vector3f()));
    }

    public DirectionalLight(Vector3f ambient, Vector3f diffuse, Vector3f specular, float shadowFrustumOffset){
        this(ambient,diffuse,specular);
        setShadowFrustumOffset(shadowFrustumOffset);
    }

    public void setShadowFrustumOffset(float frustumOffset){
        ((org.strobe.gfx.lights.DirectionalLight)light).setShadowFrustumOffset(frustumOffset);
    }

    public float getShadowFrustumOffset(){
        return ((org.strobe.gfx.lights.DirectionalLight)light).getShadowFrustumOffset();
    }
}
