package org.aspect.graphics.passes.effects;

import org.aspect.Aspect;
import org.aspect.graphics.primitives.PrimitiveIbo;
import org.aspect.graphics.primitives.PrimitiveVbo;
import org.aspect.graphics.bindable.*;
import org.aspect.graphics.passes.BindingPass;
import org.aspect.graphics.renderer.Renderer;
import org.aspect.graphics.renderer.graph.Source;
import org.aspect.graphics.renderer.DrawMode;
import org.aspect.graphics.drawable.attachments.TextureAttachment;

import static org.lwjgl.opengl.GL11.GL_DEPTH_TEST;
import static org.lwjgl.opengl.GL11.glDisable;

public class PostProcessingPass extends BindingPass {

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

        ppShader = Aspect.loadShader("assets/shaders/pp.vs", "assets/shaders/pp.fs");

        screenVao = Aspect.createVertexArray(
                true, new PrimitiveIbo.FillQuad(),
                true,
                new PrimitiveVbo.QuadPositions(2, 2),
                new PrimitiveVbo.QuadUVs());

    }

    @Override
    public void setupSinksAndSources() {
        screen = registerSinkSource(Framebuffer.class, SCREEN);
        target = registerSinkSource(Framebuffer.class, TARGET);
        gbuffer = registerSinkSource(Framebuffer.class,GBUFFER);
    }

    @Override
    public void complete() {
        addBindable(screen.get());
        addBindable(ppShader);
        screenTex = target.get().getColorAttachment(0).makeAttachment(ppShader,"screen");
        depthTex = target.get().getDepthMap().makeAttachment(ppShader,"depthMap");
    }

    @Override
    public void _execute() {
        screenTex.attach(ppShader, renderer);
        depthTex.attach(ppShader,renderer);
        screenVao.bind();
        renderer.setDrawMode(DrawMode.TRIANGLES);
        renderer.setDrawCount(6);
        glDisable(GL_DEPTH_TEST);
        renderer.executeDrawCall();
    }
}
