package org.aspect.graphics.renderer;

import org.aspect.Aspect;
import org.aspect.graphics.bindable.Framebuffer;
import org.aspect.graphics.bindable.Texture;
import org.aspect.graphics.renderer.graph.RenderGraph;

import java.util.ArrayList;
import java.util.List;

import static org.lwjgl.opengl.GL11.*;

public final class FrameGraph {

    private final Framebuffer back_buffer;

    private final List<RenderGraph> graphs = new ArrayList<>();
    private final List<Integer> priorities = new ArrayList<>();

    public FrameGraph(Renderer renderer) {

        back_buffer = Aspect.createFramebuffer(
                renderer.getWindow().getWidth(),
                renderer.getWindow().getHeight());
        Texture back_buffer_texture = Aspect.createTexture(
                null, renderer.getWindow().getWidth(),
                renderer.getWindow().getHeight(), 1,
                false, Texture.Format.RGBA,
                Texture.Type.UNSIGNED_BYTE);
        back_buffer.attachColorAttachment(0,
                back_buffer_texture, true);
        back_buffer.attachDepth24Stencil8Attachment(1);
        back_buffer.complete();

    }

    public RenderGraph attachGraph(RenderGraph graph, int priority) {
        graph.registerGlobalResource("back_buffer",
                Framebuffer.class, back_buffer);

        for (int i = 0; i < graphs.size(); i++) {
            if (priorities.get(i) > priority) {
                priorities.add(i, priority);
                graphs.add(i, graph);
                return graph;
            }
        }

        priorities.add(priority);
        graphs.add(graph);
        return graph;
    }

    public void render() {

        back_buffer.bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        back_buffer.unbind();

        glEnable(GL_TEXTURE_2D);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glDepthFunc(GL_LESS);

        back_buffer.bind();
        //render all graphs to the back_buffer
        for (RenderGraph graph : graphs) {
            graph.render();
        }
        back_buffer.unbind();
    }

    public Framebuffer getBackBuffer() {
        return back_buffer;
    }

    public void link() {
        for (RenderGraph graph : graphs) {
            graph.link();
        }
    }

    public void complete() {
        for (RenderGraph graph : graphs) {
            graph.complete();
        }

    }
}
