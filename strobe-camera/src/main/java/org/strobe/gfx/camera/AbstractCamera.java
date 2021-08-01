package org.strobe.gfx.camera;

import org.joml.Matrix4f;
import org.joml.Vector3f;
import org.joml.Vector4f;
import org.strobe.gfx.transform.AbstractTransform;
import org.strobe.gfx.transform.IdentityTransform;
import org.strobe.gfx.transform.RenderTransform3D;

public abstract class AbstractCamera {

    public static final int FXAA = 1;

    protected final Matrix4f viewMatrix = new Matrix4f().identity();
    protected final Matrix4f projMatrix = new Matrix4f().identity();
    protected final Vector3f position = new Vector3f();

    private final int horResolution;
    private final int verResolution;

    private final CameraSetting settings = new CameraSetting();

    protected final AbstractTransform root = new IdentityTransform();

    public AbstractCamera(int horResolution, int verResolution) {
        this.horResolution = horResolution;
        this.verResolution = verResolution;
    }

    public void updateViewMatrix() {
        Matrix4f transform = new Matrix4f(root.getTransformationMatrix());
        Vector4f temp = new Vector4f(0,0,0,1);
        transform.transform(temp);
        position.set(temp.x, temp.y, temp.z);

        viewMatrix.set(transform.invertAffine());
    }

    public void enable(int setting){
        settings.enable(setting);
    }

    public void disable(int setting){
        settings.disable(setting);
    }

    public boolean isEnabled(int setting){
        return settings.isEnabled(setting);
    }

    public Matrix4f getViewMatrix() {
        return viewMatrix;
    }

    public Matrix4f getProjMatrix() {
        return projMatrix;
    }

    public Vector3f getPosition(){
        return position;
    }

    public AbstractTransform getRootTransform() {
        return root;
    }


    public int getHorResolution() {
        return horResolution;
    }

    public int getVerResolution() {
        return verResolution;
    }
}
