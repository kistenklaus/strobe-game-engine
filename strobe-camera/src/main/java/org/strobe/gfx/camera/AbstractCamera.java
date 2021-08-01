package org.strobe.gfx.camera;

import org.joml.Matrix4f;
import org.joml.Vector3f;
import org.strobe.gfx.Graphics;
import org.strobe.gfx.camera.filters.FXAAFilter;
import org.strobe.gfx.opengl.bindables.framebuffer.Framebuffer;

import java.util.ArrayList;

public abstract class AbstractCamera {

    private static final Framebuffer.Attachment[] DEFAULT_ATTACHMENTS = {
            Framebuffer.Attachment.COLOR_RGBA_ATTACHMENT_0,
            Framebuffer.Attachment.DEPTH_ATTACHMENT
    };

    public static final int FXAA = 1;

    protected final Matrix4f viewMatrix = new Matrix4f().identity();
    protected final Matrix4f projMatrix = new Matrix4f().identity();
    protected final Vector3f position = new Vector3f();

    private Framebuffer frontTarget;
    private Framebuffer backTarget;

    private final CameraUbo ubo;

    private final Vector3f backgroundColor = new Vector3f(0);

    private final ArrayList<CameraFilter> filters = new ArrayList<>();

    private boolean linearScalingEnabled = false;

    private FXAAFilter fxaaFilter = null;
    private boolean fxaaEnabled = false;

    public AbstractCamera(Graphics gfx, int horResolution, int verResolution) {
        frontTarget = new Framebuffer(gfx, horResolution, verResolution, DEFAULT_ATTACHMENTS);
        backTarget = new Framebuffer(gfx, horResolution, verResolution, DEFAULT_ATTACHMENTS);
        ubo = new CameraUbo(gfx);
    }

    public void enableLinearScaling(){
        linearScalingEnabled = true;
    }

    public void disableLinearScaling(){
        linearScalingEnabled = false;
    }

    public void enableFXAA(Graphics gfx){
        fxaaFilter = new FXAAFilter(gfx);
        fxaaEnabled = true;
        addFilter(fxaaFilter);
    }

    public void disableFXAA(){
        fxaaEnabled = false;
        if(fxaaFilter == null)return;
        removeFilter(fxaaFilter);
    }

    public abstract void update();

    public Matrix4f getViewMatrix() {
        return viewMatrix;
    }

    public Matrix4f getProjMatrix() {
        return projMatrix;
    }

    public final void swapBuffers(){
        Framebuffer temp = frontTarget;
        frontTarget = backTarget;
        backTarget = temp;
    }

    public Framebuffer getTarget(){
        return frontTarget;
    }

    public Framebuffer getBackTarget(){
        return backTarget;
    }

    public CameraUbo getCameraUbo() {
        return ubo;
    }

    public Vector3f getPosition(){
        return position;
    }

    public Vector3f getBackgroundColor(){
        return backgroundColor;
    }

    public void addFilter(CameraFilter filter) {
        filters.add(filter);
    }

    public void removeFilter(CameraFilter filter){
        filters.remove(filter);
    }

    public Iterable<CameraFilter> filters(){
        return filters;
    }

    public boolean isEnabledLinearScaling() {
        return linearScalingEnabled;
    }

    public boolean isEnabledFXAA(){
        return fxaaEnabled;
    }
}
