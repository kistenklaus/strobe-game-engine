package org.strobe.ecs.context.renderer.camera;

import org.joml.Matrix4f;

public final class PerspectiveLens extends CameraLense {

    private float fov;
    private float aspect;
    private float near;
    private float far;
    private Matrix4f proj;

    public PerspectiveLens(float fov, float aspect, float near, float far) {
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

    public void setNear(float near){
        this.near = near;
    }

    @Override
    public float getFar() {
        return far;
    }

    public void setFar(float far){
        this.far = far;
    }

    public void setAspect(float aspect){
        this.aspect = aspect;
    }

    public float getAspect(){
        return aspect;
    }

    public void setFov(float fov){
        this.fov = fov;
    }

    public float getFov(){
        return fov;
    }
}

