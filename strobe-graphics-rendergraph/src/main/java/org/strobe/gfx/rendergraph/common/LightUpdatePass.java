package org.strobe.gfx.rendergraph.common;

import org.strobe.gfx.Graphics;
import org.strobe.gfx.rendergraph.common.manager.LightManager;
import org.strobe.gfx.rendergraph.core.RenderPass;
import org.strobe.gfx.rendergraph.core.Resource;

public final class LightUpdatePass extends RenderPass {

    private final Resource<LightManager> lights = registerResource(LightManager.class, "lights");

    @Override
    protected void complete(Graphics gfx) {
        if(lights.get() == null)throw new IllegalArgumentException();
    }

    @Override
    protected void reset(Graphics gfx) {

    }

    @Override
    protected void render(Graphics gfx) {
        lights.get().updateUbos(gfx);
    }

    @Override
    protected void dispose(Graphics gfx) {

    }

    public Resource<LightManager> getLightResource(){
        return lights;
    }
}
