package org.aspect.graphics.passes.sources;

import org.aspect.Aspect;
import org.aspect.graphics.bindable.UniformBuffer;
import org.aspect.graphics.data.Camera;
import org.aspect.graphics.data.CameraBuffer;
import org.aspect.graphics.passes.BindingPass;
import org.aspect.graphics.passes.Pass;

public class CameraSourcePass extends Pass {

    public static final String CAMERA = "camera";

    private final CameraBuffer cameraBuffer;

    public CameraSourcePass(String name, int bindingIndex, Camera camera) {
        super(name);
        UniformBuffer ubo = Aspect.createUniformBuffer(bindingIndex,
                "mat4 view", "mat4 proj", "vec3 position");
        this.cameraBuffer = new CameraBuffer(camera,ubo);

        camera.updateProj();
        this.cameraBuffer.getUbo().uniformMat4("proj", camera.getProjection());

    }

    @Override
    public void setupSinksAndSources() {
        registerSource(CameraBuffer.class, CAMERA, cameraBuffer);
    }

    public void complete(){}

    @Override
    public void execute() {
        cameraBuffer.getCamera().updateView();
        cameraBuffer.getCamera().updateProj();
        cameraBuffer.getUbo().uniformMat4("view", cameraBuffer.getCamera().getView());
        cameraBuffer.getUbo().uniformVec3("position", cameraBuffer.getCamera().getPosition());
    }
}
