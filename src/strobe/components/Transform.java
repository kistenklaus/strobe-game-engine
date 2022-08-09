package strobe.components;

import marshal.MarshalComponent;
import org.joml.Matrix4f;
import org.joml.Vector3f;
import strobe.debug.DragInDebug;

public class Transform extends MarshalComponent {

    @DragInDebug(speed = "0.01")
    public final Vector3f position;
    @DragInDebug(speed = "1")
    public final Vector3f rotation;
    @DragInDebug(speed = "0.01")
    public final Vector3f rotation_center;
    @DragInDebug(speed = "0.01")
    public final Vector3f scale;


    private final Matrix4f transform;

    public Transform() {
        this(new Vector3f(), new Vector3f(1,1,1), new Vector3f(), new Vector3f());
    }

    public Transform(Vector3f position){
        this(position, new Vector3f(1,1,1), new Vector3f(), new Vector3f());
    }

    public Transform(Vector3f position, Vector3f scale){
        this(position, scale, new Vector3f(), new Vector3f());
    }

    public Transform(Vector3f position, Vector3f scale, Vector3f rotation){
        this(position, scale, rotation, new Vector3f());
    }

    public Transform(Vector3f position, Vector3f scale, Vector3f rotation, Vector3f rotation_center){
        this.position = position;
        this.rotation = rotation;
        this.rotation_center = rotation_center;
        this.scale = scale;
        this.transform = new Matrix4f().identity();
    }

    public void translate(Vector3f translation) {
        this.position.add(translation);
    }

    public void rotate(Vector3f rotation) {
        this.rotation.add(rotation);
    }

    public void scale(Vector3f scale) {
        this.scale.mul(scale);
    }

    public void setPosition(Vector3f position) {
        this.position.set(position);
    }

    public void setRotation(Vector3f rotation) {
        this.rotation.set(rotation);
    }

    public void setRotationCenter(Vector3f rotation_center) {
        this.rotation_center.set(rotation_center);
    }

    public void setScale(Vector3f scale) {
        this.scale.set(scale);
    }

    public synchronized void setTransformationMatrix(Matrix4f transform) {
        this.transform.set(transform);
    }


    public Vector3f getPosition() {
        return position;
    }

    public Vector3f getRotation() {
        return rotation;
    }

    public Vector3f getRotationCenter() {
        return rotation_center;
    }

    public Vector3f getScale() {
        return scale;
    }

    public synchronized Matrix4f getTransformationMatrix() {
        return new Matrix4f(this.transform);
    }
}
