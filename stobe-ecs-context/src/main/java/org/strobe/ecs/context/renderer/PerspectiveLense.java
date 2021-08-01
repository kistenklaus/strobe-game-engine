package org.strobe.ecs.context.renderer;

import org.joml.Matrix4f;

public final class PerspectiveLense extends CameraLense{

    private float fov;
    private float aspect;
    private float near;
    private float far;

    public PerspectiveLense(float fov, float aspect, float near, float far){
        this.fov = (float) Math.toRadians(fov);
        this.aspect = aspect;
        this.near = near;
        this.far = far;
    }

    @Override
    public void project(Matrix4f projectionMatrix) {
        projectionMatrix.perspective(fov, aspect, near, far);
    }
}

