package org.strobe.gfx.transform;

import org.joml.Matrix4f;
import org.joml.Quaternionf;
import org.joml.Vector3f;

import java.util.ArrayList;
import java.util.List;

public abstract class AbstractTransform {

    private static final RootTransform root = new RootTransform();

    public static AbstractTransform getRoot() {
        return root;
    }

    public static void updateTransforms() {
        getRoot().updateTransformationMatrix();
    }

    protected final Vector3f position;
    protected final Vector3f scale;
    protected final Quaternionf orientation;

    protected final Matrix4f transformationMatrix = new Matrix4f().identity();
    private final Matrix4f globalTransform = new Matrix4f().identity();

    private final List<AbstractTransform> children = new ArrayList<>();
    private AbstractTransform parent;

    private AbstractTransform() {
        //constructor for the root
        this.position = new Vector3f();
        this.scale = new Vector3f(1);
        this.orientation = new Quaternionf().identity();
        parent = null;
    }

    public AbstractTransform(AbstractTransform parent, Vector3f position, Vector3f scale, Quaternionf orientation) {
        if (position != null) this.position = new Vector3f(position);
        else this.position = new Vector3f();
        if (scale != null) this.scale = new Vector3f(scale);
        else this.scale = new Vector3f(1);
        if (orientation != null) this.orientation = new Quaternionf(orientation);
        else this.orientation = new Quaternionf().identity();
        if (parent == null) parent = getRoot();
        this.parent = parent;
        parent.children.add(this);
    }

    private synchronized void updateTransformationSceneGraph() {
        globalTransform.set(parent.globalTransform);
        updateTransformationMatrix();
        globalTransform.mul(transformationMatrix);
        for (AbstractTransform child : children) {
            child.updateTransformationSceneGraph();
        }
    }

    public void relocate(AbstractTransform newParent) {
        if(newParent == null)newParent = getRoot();
        if (newParent == parent) return;
        parent.children.remove(this);
        parent = newParent;
        parent.children.add(this);
    }

    public abstract void updateTransformationMatrix();


    public synchronized Matrix4f getTransformationMatrix() {
        return globalTransform;
    }

    public synchronized AbstractTransform getParent() {
        return parent;
    }

    public synchronized Vector3f getPosition() {
        return position;
    }

    public synchronized float getX() {
        return position.x;
    }

    public synchronized float getY() {
        return position.y;
    }

    public synchronized float getZ() {
        return position.z;
    }

    public synchronized void setPosition(Vector3f position) {
        this.position.set(position);
    }

    public synchronized void setX(float x) {
        this.position.x = x;
    }

    public synchronized void setY(float y) {
        this.position.y = y;
    }

    public synchronized void setZ(float z) {
        this.position.z = z;
    }

    public void translate(Vector3f translation) {
        position.add(translation);
    }

    public synchronized Vector3f getScale() {
        return scale;
    }

    public synchronized void scale(Vector3f scale){
        this.scale.mul(scale);
    }

    public synchronized float getSX() {
        return scale.x;
    }

    public synchronized float getSY() {
        return scale.y;
    }

    public synchronized float getSZ() {
        return scale.z;
    }

    public synchronized void setScale(Vector3f scale) {
        this.scale.set(scale);
    }

    public synchronized void setSX(float sx) {
        this.scale.x = sx;
    }

    public synchronized void setSY(float sy) {
        this.scale.y = sy;
    }

    public synchronized void setSZ(float sz) {
        this.scale.z = sz;
    }

    public synchronized Quaternionf getOrientation() {
        return orientation;
    }

    public synchronized void setOrientation(Quaternionf orientation){
        if(this.orientation == orientation)return;
        this.orientation.set(orientation);
    }

    public synchronized Vector3f getForward() {
        return orientation.transform(new Vector3f(0, 0, -1));
    }

    public synchronized Vector3f getRight() {
        return orientation.transform(new Vector3f(1, 0, 0));
    }

    public synchronized Vector3f getUp() {
        return orientation.transform(new Vector3f(0, 1, 0));
    }

    public synchronized Vector3f getBack() {
        return orientation.transform(new Vector3f(0, 0, 1));
    }

    public synchronized Vector3f getLeft() {
        return orientation.transform(new Vector3f(-1, 0, 0));
    }

    public synchronized Vector3f getDown() {
        return orientation.transform(new Vector3f(0, -1, 0));
    }

    public synchronized void rotateX(float drx) {
        orientation.rotateX(drx);
    }

    public synchronized void rotateY(float dry) {
        orientation.rotateY(dry);
    }

    public synchronized void rotateZ(float drz) {
        orientation.rotateZ(drz);
    }

    public synchronized void rotateAxis(float angle, Vector3f axis){
        orientation.rotateAxis(angle, axis);
    }


    private static final class RootTransform extends AbstractTransform {

        @Override
        public synchronized void updateTransformationMatrix() {
            getRoot().globalTransform.identity();
            for (AbstractTransform child : getRoot().children) {
                child.updateTransformationSceneGraph();
            }
        }

        @Override
        public void relocate(AbstractTransform abstractTransform) {
            throw new UnsupportedOperationException();
        }
    }

}
