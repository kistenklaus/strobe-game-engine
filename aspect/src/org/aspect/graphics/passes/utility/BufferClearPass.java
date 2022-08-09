package org.aspect.graphics.passes.utility;

import org.aspect.graphics.bindable.Framebuffer;
import org.aspect.graphics.passes.BindingPass;
import org.aspect.graphics.renderer.graph.Source;
import org.joml.Vector4f;

import static org.lwjgl.opengl.GL11.*;

public class BufferClearPass extends BindingPass {

    public static final String TARGET = "target";

    private Source<Framebuffer> buffer;
    private Vector4f clearColor;

    public BufferClearPass(String name) {
        this(name, new Vector4f(0));
    }

    public BufferClearPass(String name, Vector4f clearColor) {
        super(name);
        this.clearColor = clearColor;
    }

    @Override
    public void setupSinksAndSources() {
        buffer = registerSinkSource(Framebuffer.class,TARGET);
    }

    @Override
    public void completeBindable() {
        addBindable(buffer.get());
    }

    @Override
    public void _execute() {
        glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }
}
