package org.aspect.graphics.passes.effects;

import org.aspect.Aspect;
import org.aspect.graphics.Graphics;
import org.aspect.graphics.primitives.PrimitiveIboContent;
import org.aspect.graphics.primitives.PrimitiveVboContent;
import org.aspect.graphics.bindable.*;
import org.aspect.graphics.passes.BindingPass;
import org.aspect.graphics.renderer.Renderer;
import org.aspect.graphics.renderer.graph.Source;
import org.aspect.graphics.renderer.DrawMode;
import org.aspect.graphics.drawable.attachments.TextureAttachment;

public class PostProcessingPass extends BindingPass {

    private static final String VERTEX_SHADER_LOC = Graphics.SHADERS_LOCATION + "post-processing.vs";
    private static final String FRAGMENT_SHADER_LOC = Graphics.SHADERS_LOCATION + "post-processing.fs";

    public static final String SCREEN = "screen";
    public static final String TARGET = "target";
    public static final String GBUFFER = "gbuffer";

    private final Renderer renderer;

    private final Shader ppShader;
    private final VertexArray screenVao;

    private Source<Framebuffer> target;
    private Source<Framebuffer> screen;
    //note that the gbuffers only stores depth or color of deferred techniques
    private Source<Framebuffer> gbuffer;
    private TextureAttachment screenTex;
    private TextureAttachment depthTex;


    public PostProcessingPass(String name, Renderer renderer) {
        super(name);
        this.renderer = renderer;

        ppShader = Aspect.loadShaderFromClasspath(VERTEX_SHADER_LOC, FRAGMENT_SHADER_LOC);

        screenVao = Aspect.createVertexArray(
                true, new PrimitiveIboContent.FillQuad(),
                true,
                new PrimitiveVboContent.QuadPositions(2, 2),
                new PrimitiveVboContent.QuadUVs());

    }

    @Override
    public void setupSinksAndSources() {
        screen = registerSinkSource(Framebuffer.class, SCREEN);
        target = registerSinkSource(Framebuffer.class, TARGET);
        gbuffer = registerSinkSource(Framebuffer.class,GBUFFER);
    }

    @Override
    public void completeBindable() {
        addBindable(screen.get());
        addBindable(ppShader);
        screenTex = target.get().getColorAttachment(0).makeAttachment(ppShader,"screen");
        depthTex = target.get().getDepthMap().makeAttachment(ppShader,"depthMap");
    }

    @Override
    public void _execute() {
        ppShader.uniformVec2("invScreenSize", 1.0f/screen.get().getWidth(), 1.0f/screen.get().getHeight());
        screenTex.attach(ppShader, renderer);
        depthTex.attach(ppShader,renderer);
        screenVao.bind();
        renderer.setDrawMode(DrawMode.TRIANGLES);
        renderer.setDrawCount(6);
        renderer.executeDrawCall();
    }
}
