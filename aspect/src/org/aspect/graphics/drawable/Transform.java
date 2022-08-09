package org.aspect.graphics.drawable;

import org.aspect.graphics.bindable.Shader;
import org.joml.Matrix4f;
import org.joml.Vector3f;

public final class Transform{

    private final Vector3f position = new Vector3f();
    private final Vector3f rotation = new Vector3f();
    private final Vector3f scale = new Vector3f(1);
    private final Matrix4f model = new Matrix4f();
    private boolean modelUpdated = false;

    public Transform() {
    }

    public void updateModelMatrix() {
        if (!modelUpdated) return;
        model.identity();
        model.translate(position);
        model.scale(scale);
        model.rotate(rotation.x, 1, 0, 0);
        model.rotate(rotation.y, 0, 1, 0);
        model.rotate(rotation.z, 0, 0, 1);
    }

    public void bind() {
    }

    public void updateModelUniform(Shader shader) {
        updateModelMatrix();
        shader.uniformMat4("model", model);
    }

    public Transform setPosition(Vector3f position) {
        this.position.set(position);
        modelUpdated = true;
        return this;
    }

    public Transform translate(Vector3f translation){
        this.position.add(translation);
        modelUpdated = true;
        return this;
    }

    public Transform setRotation(Vector3f rotation) {
        this.rotation.set(rotation);
        modelUpdated = true;
        return this;
    }

    public Transform setScale(Vector3f scale) {
        this.scale.set(scale);
        modelUpdated = true;
        return this;
    }

    public Transform setScale(float scale){
        this.scale.set(scale);
        modelUpdated = true;
        return this;
    }

    public Transform rotateX(float dx) {
        rotation.x += dx;
        modelUpdated = true;
        return this;
    }

    public Transform rotateY(float dy){
        rotation.y += dy;
        modelUpdated = true;
        return this;
    }

    public Transform rotateZ(float dz){
        rotation.z += dz;
        modelUpdated = true;
        return this;
    }

    public Vector3f getPosition(){
        return position;
    }

    public float getPosX(){
        return position.x;
    }

    public float getPosY(){
        return position.y;
    }
    public float getPosZ(){
        return position.z;
    }
}
