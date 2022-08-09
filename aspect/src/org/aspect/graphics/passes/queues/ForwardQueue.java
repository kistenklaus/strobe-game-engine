package org.aspect.graphics.passes.queues;

import org.aspect.graphics.bindable.Framebuffer;
import org.aspect.graphics.bindable.UniformBuffer;
import org.aspect.graphics.renderer.graph.Source;

import static org.lwjgl.opengl.GL11.*;
import static org.lwjgl.opengl.GL30.*;

public class ForwardQueue extends RenderQueuePass {


    public static final String TARGET = "target";
    public static final String GBUFFER = "gbuffer";
    public static final String CAMERA_UBO = "cameraUbo";
    public static final String LIGHT_UBO = "lightUbo";

    private Source<Framebuffer> target;
    private Source<Framebuffer> gbuffer;
    private Source<UniformBuffer> cameraUbo;
    private Source<UniformBuffer> lightUbo;

    public ForwardQueue(String name) {
        super(name);
    }

    @Override
    public void setupSinksAndSources() {
        target = registerSinkSource(Framebuffer.class, TARGET);
        gbuffer = registerSinkSource(Framebuffer.class, GBUFFER);
        cameraUbo = registerSinkSource(UniformBuffer.class, CAMERA_UBO);
        lightUbo = registerSinkSource(UniformBuffer.class, LIGHT_UBO);

    }

    @Override
    public void complete() {
        addBindable(cameraUbo.get());
        addBindable(lightUbo.get());
        addBindable(target.get());
    }


    @Override
    public void prepare() {
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
    }
}
