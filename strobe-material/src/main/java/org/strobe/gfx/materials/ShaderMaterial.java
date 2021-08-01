package org.strobe.gfx.materials;

import org.strobe.gfx.Graphics;

public abstract class ShaderMaterial{

    private final MaterialPass[] passes;

    public ShaderMaterial(Graphics gfx){
        passes = createPasses(gfx);
    }

    protected abstract MaterialPass[] createPasses(Graphics gfx);

    public MaterialPass[] passes(){
        return passes;
    }

    public int passCount() {
        return passes.length;
    }
}
