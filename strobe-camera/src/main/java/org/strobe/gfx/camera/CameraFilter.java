package org.strobe.gfx.camera;

import org.strobe.gfx.Graphics;
import org.strobe.gfx.opengl.bindables.framebuffer.Framebuffer;

public interface CameraFilter {

    void beforeFSR(Graphics gfx, Framebuffer source);

    void afterFSR(Graphics gfx);
}
