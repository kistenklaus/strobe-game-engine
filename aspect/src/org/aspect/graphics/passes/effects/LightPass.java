package org.aspect.graphics.passes.effects;

import org.aspect.Aspect;
import org.aspect.graphics.Graphics;
import org.aspect.graphics.data.CameraBuffer;
import org.aspect.graphics.primitives.PrimitiveIboContent;
import org.aspect.graphics.primitives.PrimitiveVboContent;
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

public class LightPass extends BindingPass {

    private static final String VERTEX_SHADER_LOC = Graphics.SHADERS_LOCATION + "deferredLightPass.vs";
    private static final String FRAGMENT_SHADER_LOC = Graphics.SHADERS_LOCATION + "deferredLightPass.fs";

    public static final String TARGET = "target";
    public static final String GBUFFER = "gbuffer";
    public static final String CAMERA = "camera";
    public static final String LIGHT_UBO = "lightUbo";

    private final Shader lightShader;
    private final VertexArray screenVao;

    private final Renderer renderer;

    private Source<Framebuffer> target;
    private Source<Framebuffer> gbuffer;

    private Source<CameraBuffer> camera;
    private Source<UniformBuffer> lightUbo;

    private TextureAttachment positions_attachment;
    private TextureAttachment normals_attachment;
    private TextureAttachment albedoSpecAttachment;

    public LightPass(String name, Renderer renderer) {
        super(name);
        this.renderer = renderer;
        lightShader = Aspect.loadShaderFromClasspath(VERTEX_SHADER_LOC, FRAGMENT_SHADER_LOC);
        screenVao = Aspect.createVertexArray(true, new PrimitiveIboContent.FillQuad(),
                true, new PrimitiveVboContent.QuadPositions(2, 2),
                new PrimitiveVboContent.QuadUVs());
    }

    @Override
    public void setupSinksAndSources() {
        target = registerSinkSource(Framebuffer.class, TARGET);
        gbuffer = registerSinkSource(Framebuffer.class, GBUFFER);
        camera = registerSinkSource(CameraBuffer.class, CAMERA);
        lightUbo = registerSinkSource(UniformBuffer.class, LIGHT_UBO);
    }

    @Override
    public void completeBindable() {
        addBindable(camera.get().getUbo());
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

        renderer.executeDrawCall();
    }
}
