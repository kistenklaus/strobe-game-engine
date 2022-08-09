package org.aspect.graphics.renderer;

import org.aspect.Aspect;
import org.aspect.graphics.primitives.PrimitiveIbo;
import org.aspect.graphics.primitives.PrimitiveVbo;
import org.aspect.graphics.Graphics;
import org.aspect.graphics.bindable.Framebuffer;
import org.aspect.graphics.bindable.Shader;
import org.aspect.graphics.bindable.VertexArray;
import org.aspect.graphics.renderer.graph.RenderGraph;
import org.aspect.graphics.drawable.Drawable;
import org.aspect.graphics.drawable.attachments.TextureAttachment;
import org.aspect.window.Window;

import static org.lwjgl.opengl.GL11.*;
import static org.lwjgl.opengl.GL30.*;
import static org.lwjgl.opengl.GL31.glDrawElementsInstanced;

public final class Renderer {

    private final Window window;

    private final Graphics graphics;

    private final VertexArray screenVao;
    private final Shader fullScreenShader;
    private final FrameGraph frameGraph;
    private Framebuffer back_buffer;
    private TextureAttachment back_buffer_attachment;
    private DrawMethod drawMethod;
    private int drawMode;
    private int drawCount;
    private int instanceCount;

    public Renderer(Window window, Graphics graphics) {
        this.window = window;
        this.graphics = graphics;

        this.frameGraph = new FrameGraph(this);

        this.back_buffer = frameGraph.getBackBuffer();

        screenVao = Aspect.createVertexArray(
                true, new PrimitiveIbo.FillCube(),
                true,
                new PrimitiveVbo.QuadPositions(2, 2),
                new PrimitiveVbo.QuadUVs());


        fullScreenShader = Aspect.loadShaderFromClasspath(
                "/org/aspect/graphics/shader/fullScreen.vs",
                "/org/aspect/graphics/shader/fullScreen.fs");

        this.back_buffer_attachment = back_buffer
                .getColorAttachment(0).makeAttachment(
                        fullScreenShader, "screen");

        resetDrawSettings();
    }

    public void blitFramebuffer(Framebuffer read, Framebuffer draw, BufferBits blitTarget, InterpMode interpMode) {
        glBindFramebuffer(GL_READ_FRAMEBUFFER,read.getID());
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER,draw.getID());
        glBlitFramebuffer(0,0,read.getWidth(),read.getHeight(),0,0,
                draw.getWidth(),draw.getHeight(),blitTarget.getGlID(), interpMode.getGlID());
        glBindFramebuffer(GL_FRAMEBUFFER,0);
    }

    public void executeDrawCall() {
        switch (drawMethod) {
            case ELEMENTS -> glDrawElements(drawMode, drawCount, GL_UNSIGNED_INT, 0);
            case INSTANCED_ELEMENTS -> glDrawElementsInstanced(
                    drawMode, drawCount, GL_UNSIGNED_INT, 0, instanceCount);

        }
    }

    public Framebuffer getBackBuffer() {
        return back_buffer;
    }

    public void resetDrawSettings() {
        this.drawMethod = DrawMethod.ELEMENTS;
        this.drawMode = GL_TRIANGLES;
    }

    public void setDrawMode(DrawMode mode) {
        this.drawMode = mode.getDrawMode();
    }

    public void setDrawCount(int drawCount) {
        this.drawCount = drawCount;
    }

    public void setDrawMethod(DrawMethod method) {
        this.drawMethod = method;
    }

    public void setInstanceCount(int instanceCount) {
        this.instanceCount = instanceCount;
    }


    /*
    -----------Frame-Graph-------------
     */

    public void addGraph(RenderGraph graph){
        addGraph(graph,0);
    }
    public void addGraph(RenderGraph graph, int priority) {
        frameGraph.attachGraph(graph, priority);
    }

    public void link() {
        frameGraph.link();
    }

    public void complete() {
        frameGraph.complete();
    }

    public void draw(Drawable drawable, RenderGraph graph) {
        drawable.submit(window.getRenderer(), graph);
    }

    public void render() {

        resetDrawSettings();

        frameGraph.render();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);


        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);

        fullScreenShader.bind();
        back_buffer_attachment.bind();
        back_buffer_attachment.prepareShader(fullScreenShader);

        screenVao.bind();
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        screenVao.unbind();

        back_buffer_attachment.unbind();
        fullScreenShader.unbind();


    }

    public Window getWindow() {
        return window;
    }
}
