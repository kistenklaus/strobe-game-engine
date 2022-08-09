package org.aspect.graphics.data;

import org.joml.Matrix4f;
import org.joml.Vector3f;

import java.util.function.Supplier;

public final class Camera {

    private static final float DEFAULT_FOV = 60;

    private final ProjectionMode projectionMode;
    private final CameraMode cameraMode;

    private final Vector3f position = new Vector3f();
    private final Vector3f rotation = new Vector3f();

    private final Vector3f forward = new Vector3f(0, 0, -1);
    private final Vector3f right = new Vector3f(1, 0, 0);
    private final Vector3f up = new Vector3f(0, 1, 0);

    private final Matrix4f view = new Matrix4f();
    private final Matrix4f proj = new Matrix4f();

    private float fov = DEFAULT_FOV;
    private float width;
    private float height;
    private float near;
    private float far;

    private boolean viewChange = false;
    private boolean dirChange = false;
    private boolean projChange = false;

    public Camera(CameraMode cameraMode, float width, float height, float near, float far) {
        this.cameraMode = cameraMode;
        this.projectionMode = cameraMode.getProjMode();

        this.width = width;
        this.height = height;
        this.near = near;
        this.far = far;

        updateView();
        updateProj();
    }

    public void updateView() {
        if (viewChange) {
            if (dirChange) {
                computeDirections();
                dirChange = false;
            }
            view.set(right.x, up.x, -forward.x, 0,
                    right.y, up.y, -forward.y, 0,
                    right.z, up.z, -forward.z, 0,
                    0, 0, 0, 1);
            view.translate(-position.x, -position.y, -position.z);
            viewChange = false;
        }
    }

    private void computeDirections() {
        Vector3f worldUp = new Vector3f(0, 1, 0);
        right.set(worldUp.cross(forward));
        up.set(new Vector3f(forward).cross(right));
        right.mul(-1);
        forward.normalize();
        right.normalize();
        up.normalize();
    }

    public Camera pitch(float angle) {
        this.forward.rotateAxis((float) Math.toRadians(angle), up.x, up.y, up.z);
        dirChange = true;
        viewChange = true;
        return this;
    }

    public Camera roll(float angle) {
        this.forward.rotateAxis((float) Math.toRadians(angle),
                right.x, right.y, right.z);
        dirChange = true;
        viewChange = true;
        return this;
    }

    public Camera dolly(float delta) {
        this.position.add(forward.x * delta, forward.y * delta, forward.z * delta);
        viewChange = true;
        return this;
    }

    public Camera truck(float delta) {
        this.position.add(right.x * delta, right.y * delta, right.z * delta);
        viewChange = true;
        return this;
    }

    public Camera pedestal(float delta) {
        this.position.add(up.x * delta, up.y * delta, up.y * delta);
        viewChange = true;
        return this;
    }

    public Camera move(float x, int y, float z) {
        this.position.add(x,y,z);
        viewChange = true;
        return this;
    }

    public Camera move(Vector3f translation){
        this.position.add(translation);
        viewChange = true;
        return this;
    }



    public void updateProj() {
        if (projChange) {
            projectionMode.updateProjection(this, proj);
            projChange = false;
        }

    }

    public Camera setPosition(float x, float y, float z) {
        this.position.set(x, y, z);
        viewChange = true;
        return this;
    }

    public Camera setRotation(float x, float y, float z) {
        this.rotation.set(x, y, z);
        viewChange = true;
        return this;
    }

    public Camera setFOV(float fov) {
        this.fov = fov;
        if (!cameraMode.hasFov) {
            System.out.println("changing the fov of a " + cameraMode + " Camera will not change shit!");
        }
        projChange = true;
        return this;
    }

    public Camera setNearPlane(float near) {
        this.near = near;
        projChange = true;
        return this;
    }

    public Camera setFarPlane(float far) {
        this.far = far;
        projChange = true;
        return this;
    }

    public Camera setRotation(Vector3f rotation) {
        this.rotation.set(rotation);
        viewChange = true;
        return this;
    }

    /**
     * please don't apply changes to this matrix!
     *
     * @return the projection matrix of the camera
     */
    public Matrix4f getProjection() {
        return proj;
    }

    /**
     * please don't apply changes to this matrix!
     *
     * @return the view matrix of the camera
     */
    public Matrix4f getView() {
        return view;
    }

    /**
     * please don't apply changes to this vector
     *
     * @return the position of the camera as a vector
     */
    public Vector3f getPosition() {
        return position;
    }

    public Camera setPosition(Vector3f position) {
        this.position.set(position);
        viewChange = true;
        return this;
    }

    public float getPosX() {
        return position.x;
    }

    public float getPosY() {
        return position.y;
    }

    public float getPosZ() {
        return position.z;
    }

    public float getRotX() {
        return rotation.x;
    }

    public float getRotY() {
        return rotation.y;
    }

    public float getRotZ() {
        return rotation.z;
    }

    public float getFov() {
        return fov;
    }

    public float getWidth() {
        return width;
    }

    public Camera setWidth(float width) {
        this.width = width;
        projChange = true;
        return this;
    }

    public float getHeight() {
        return height;
    }

    public Camera setHeight(float height) {
        this.height = height;
        projChange = true;
        return this;
    }

    public float getNear() {
        return near;
    }

    public float getFar() {
        return far;
    }

    public float getAspectRatio() {
        return width/height;
    }

    public Vector3f getForward() {
        return forward;
    }

    public Vector3f getRight(){
        return right;
    }

    public Vector3f getUp(){
        return up;
    }

    public enum CameraMode {
        PERSPECTIVE(true, PerspectiveMode::new), ORTHOGRAPHIC(false, OrthographicMode::new);

        private boolean hasFov;
        private Supplier<ProjectionMode> con;

        CameraMode(boolean hasFov, Supplier<ProjectionMode> con) {
            this.hasFov = hasFov;
            this.con = con;
        }

        public ProjectionMode getProjMode() {
            return con.get();
        }
    }

    private interface ProjectionMode {
        void updateProjection(Camera camera, Matrix4f proj);
    }


    private static class PerspectiveMode implements ProjectionMode {

        @Override
        public void updateProjection(Camera camera, Matrix4f proj) {
            proj.identity();
            proj.perspective(camera.fov, camera.width / camera.height, camera.near, camera.far);
        }
    }

    private static final class OrthographicMode implements ProjectionMode {

        @Override
        public void updateProjection(Camera camera, Matrix4f proj) {
            float w2 = camera.width / 2.0f;
            float h2 = camera.height / 2.0f;
            proj.identity();
            proj.ortho(-w2, w2, -h2, h2, camera.near, camera.far);
        }
    }
}
