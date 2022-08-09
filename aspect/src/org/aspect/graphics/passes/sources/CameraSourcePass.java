package org.aspect.graphics.passes.sources;

import org.aspect.Aspect;
import org.aspect.graphics.bindable.UniformBuffer;
import org.aspect.graphics.data.Camera;
import org.aspect.graphics.passes.BindingPass;

public class CameraSourcePass extends BindingPass {

    public static final String CAMERA_UBO = "cameraUbo";

    private final UniformBuffer cameraUbo;

    private Camera camera;

    public CameraSourcePass(String name, int bindingIndex, Camera camera) {
        super(name);
        this.camera = camera;
        cameraUbo = Aspect.createUniformBuffer(bindingIndex,
                "mat4 view", "mat4 proj", "vec3 position");


        camera.updateProj();
        cameraUbo.uniformMat4("proj", camera.getProjection());

    }

    @Override
    public void setupSinksAndSources() {
        registerSource(UniformBuffer.class, CAMERA_UBO, cameraUbo);
    }

    @Override
    public void complete() {
    }

    @Override
    public void _execute() {
        camera.updateView();
        camera.updateProj();
        cameraUbo.uniformMat4("view", camera.getView());
        cameraUbo.uniformVec3("position", camera.getPosition());
    }
}
