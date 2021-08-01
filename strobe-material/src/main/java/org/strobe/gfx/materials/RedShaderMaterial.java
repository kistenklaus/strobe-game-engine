package org.strobe.gfx.materials;

import org.strobe.gfx.Bindable;
import org.strobe.gfx.Graphics;
import org.strobe.gfx.materials.shaders.RedTestShader;

public class RedShaderMaterial extends ShaderMaterial{

    public RedShaderMaterial(Graphics gfx) {
        super(gfx);
    }

    @Override
    protected MaterialPass[] createPasses(Graphics gfx) {
        return new MaterialPass[]{new MaterialPass(new RedTestShader(gfx),0, new Bindable[]{})};
    }
}
