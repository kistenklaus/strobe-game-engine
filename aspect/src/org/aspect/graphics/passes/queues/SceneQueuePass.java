package org.aspect.graphics.passes.queues;

import org.aspect.graphics.bindable.Framebuffer;
import org.aspect.graphics.bindable.UniformBuffer;
import org.aspect.graphics.bindable.setter.Blender;
import org.aspect.graphics.bindable.setter.DepthTester;
import org.aspect.graphics.data.CameraBuffer;
import org.aspect.graphics.drawable.Transform;
import org.aspect.graphics.renderer.graph.Source;
import org.joml.Vector3f;


public class SceneQueuePass extends RenderQueuePass {

    public static final String TARGET = "target";
    public static final String CAMERA = "camera";
    public static final String LIGHT_UBO = "lightUbo";

    protected Source<Framebuffer> target;
    protected Source<CameraBuffer> camera;
    protected Source<UniformBuffer> lightUbo;

    public SceneQueuePass(String name) {
        super(name);
    }

    @Override
    public void setupSinksAndSources() {
        target = registerSinkSource(Framebuffer.class, TARGET);
        camera = registerSinkSource(CameraBuffer.class, CAMERA);
        lightUbo = registerSinkSource(UniformBuffer.class, LIGHT_UBO);
    }

    @Override
    public void completeBindable() {
        addBindable(target.get());
        addBindable(camera.get().getUbo());
        addBindable(lightUbo.get());
    }

    public void completeSettings(){
        addBindable(new DepthTester());
    }
}
