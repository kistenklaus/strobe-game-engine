package org.aspect.graphics.passes.effects;

import org.aspect.Aspect;
import org.aspect.graphics.primitives.PrimitiveIbo;
import org.aspect.graphics.primitives.PrimitiveVbo;
import org.aspect.graphics.bindable.Framebuffer;
import org.aspect.graphics.bindable.Shader;
import org.aspect.graphics.bindable.UniformBuffer;
import org.aspect.graphics.bindable.VertexArray;
import org.aspect.graphics.passes.BindingPass;
import org.aspect.graphics.passes.sources.GBufferSourcePass;
import org.aspect.graphics.renderer.Renderer;
import org.aspect.graphics.renderer.graph.Source;
import org.aspect.graphics.renderer.DrawMode;
import org.aspect.graphics.drawable.attachments.TextureAttachment;

import static org.lwjgl.opengl.GL11.*;

public class LightPass extends BindingPass {

    public static final String TARGET = "target";
    public static final String GBUFFER = "gbuffer";
    public static final String CAMERA_UBO = "cameraUbo";
    public static final String LIGHT_UBO = "lightUbo";

    private final Shader lightShader;
    private final VertexArray screenVao;

    private final Renderer renderer;

    private Source<Framebuffer> target;
    private Source<Framebuffer> gbuffer;

    private Source<UniformBuffer> cameraUbo;
    private Source<UniformBuffer> lightUbo;

    private TextureAttachment positions_attachment;
    private TextureAttachment normals_attachment;
    private TextureAttachment albedoSpecAttachment;

    public LightPass(String name, Renderer renderer) {
        super(name);
        this.renderer = renderer;
        lightShader = Aspect.loadShader(
                "assets/shaders/light.vs",
                "assets/shaders/light.fs");
        screenVao = Aspect.createVertexArray(true, new PrimitiveIbo.FillQuad(),
                true, new PrimitiveVbo.QuadPositions(2, 2),
                new PrimitiveVbo.QuadUVs());
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
        positions_attachment = gbuffer.get()
                .getColorAttachment(GBufferSourcePass.position_location)
                .makeAttachment(lightShader, "gPositions");
        normals_attachment = gbuffer.get()
                .getColorAttachment(GBufferSourcePass.normal_location)
                .makeAttachment(lightShader, "gNormals");

        albedoSpecAttachment = gbuffer.get()
                .getColorAttachment(GBufferSourcePass.albedoSpec_location)
                .makeAttachment(lightShader, "gAlbedoSpec");

        addBindable(target.get());
        addBindable(lightShader);
    }

    @Override
    public void _execute() {

        positions_attachment.attach(lightShader, renderer);

        normals_attachment.attach(lightShader, renderer);

        albedoSpecAttachment.attach(lightShader, renderer);

        screenVao.bind();

        renderer.setDrawMode(DrawMode.TRIANGLES);
        renderer.setDrawCount(6);

        glDisable(GL_DEPTH_TEST);
        renderer.executeDrawCall();
    }
}
