package org.strobe.gfx.materials;

import org.strobe.gfx.Bindable;
import org.strobe.gfx.Graphics;
import org.strobe.gfx.materials.shaders.GreenTestShader;

public class GreenShaderMaterial extends ShaderMaterial{


    public GreenShaderMaterial(Graphics gfx) {
        super(gfx);
    }

    @Override
    protected MaterialPass[] createPasses(Graphics gfx) {
        return new MaterialPass[]{new MaterialPass(new GreenTestShader(gfx), 0, new Bindable[]{})};
    }
}
