package org.aspect.graphics.renderer.graph.passes;

import org.aspect.Aspect;
import org.aspect.graphics.bindable.Framebuffer;
import org.aspect.graphics.renderer.graph.Sink;
import org.aspect.graphics.renderer.graph.Source;
import org.joml.Vector4f;

import static org.lwjgl.opengl.GL11.*;

public class BufferClearPass extends Pass{

    private Source<Framebuffer> buffer;
    private Vector4f clearColor;

    public BufferClearPass(String name) {
        this(name, new Vector4f(0));
    }

    public BufferClearPass(String name, Vector4f clearColor) {
        super(name);
        registerSink(new Sink(Framebuffer.class, "buffer", "buffer"));
        registerSource(buffer = new Source(Framebuffer.class, "buffer"));
        this.clearColor = clearColor;
    }

    @Override
    public void complete() {

    }

    @Override
    public void execute() {
        buffer.get().bind();
        glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        buffer.get().unbind();
    }
}
