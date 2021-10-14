package org.strobe.engine.release;

import org.strobe.ecs.EntityComponentSystem;
import org.strobe.ecs.context.EntityContext;
import org.strobe.engine.StrobeEngine;
import org.strobe.gfx.Graphics;
import org.strobe.gfx.opengl.bindables.framebuffer.Framebuffer;
import org.strobe.window.WindowConfiguration;
import org.strobe.window.glfw.GlfwWindow;

import static org.lwjgl.opengl.GL11.GL_COLOR_BUFFER_BIT;

public final class ReleaseEngine extends StrobeEngine<EntityContext> {

    public ReleaseEngine(EntityContext context) {
        super(context, new GlfwWindow(context.getTitle(), context.getWidth(), context.getHeight(),
                WindowConfiguration.get()));
        context.linkEntityComponentSystem(new EntityComponentSystem());
    }

    @Override
    public void afterInit(Graphics gfx) {
        gfx.addRenderer(1, new ReleaseRenderer(gfx, context));
    }

    @Override
    protected void afterRender(Graphics gfx) {
        context.getTarget().copyTo(gfx, Framebuffer.getBackBuffer(gfx), GL_COLOR_BUFFER_BIT);
    }
}
