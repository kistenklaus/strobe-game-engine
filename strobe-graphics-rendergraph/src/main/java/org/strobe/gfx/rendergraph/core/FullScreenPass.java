package org.strobe.gfx.rendergraph.core;

import org.strobe.gfx.Graphics;
import org.strobe.gfx.renderables.opengl.ScreenVao;

public abstract class FullScreenPass extends RenderPass{

    protected final ScreenVao screen;

    public FullScreenPass(Graphics gfx){
        screen = new ScreenVao(gfx);
    }

}
