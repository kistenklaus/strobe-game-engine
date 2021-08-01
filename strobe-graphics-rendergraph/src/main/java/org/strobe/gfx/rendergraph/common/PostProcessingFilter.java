package org.strobe.gfx.rendergraph.common;

import org.strobe.gfx.Graphics;
import org.strobe.gfx.opengl.bindables.framebuffer.Framebuffer;

public interface PostProcessingFilter {

    void prepareFSR(Graphics gfx, Framebuffer source);

}
