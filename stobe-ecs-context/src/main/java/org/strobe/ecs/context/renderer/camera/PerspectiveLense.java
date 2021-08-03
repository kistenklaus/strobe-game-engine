package org.strobe.ecs.context.renderer.camera;

import org.joml.Matrix4f;
import org.joml.Vector3f;
import org.joml.Vector4f;
import org.strobe.gfx.camera.CameraFrustum;

public final class PerspectiveLense extends CameraLense {

    private float fov;
    private float aspect;
    private float near;
    private float far;
    private Matrix4f proj;

    public PerspectiveLense(float fov, float aspect, float near, float far) {
        this.fov = (float) Math.toRadians(fov);
        this.aspect = aspect;
        this.near = near;
        this.far = far;
    }

    @Override
    public void project(Matrix4f projectionMatrix) {
        projectionMatrix.perspective(fov, aspect, near, far);
        proj = projectionMatrix;
    }

    @Override
    public float getNear() {
        return near;
    }

    @Override
    public float getFar() {
        return far;
    }

}

