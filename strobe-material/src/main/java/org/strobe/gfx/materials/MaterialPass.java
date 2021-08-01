package org.strobe.gfx.materials;

import org.strobe.gfx.Bindable;
import org.strobe.gfx.materials.shaders.MaterialShader;

public final class MaterialPass {

    private final MaterialShader shader;
    private final Bindable[] bindables;
    private final int renderFlags;

    public MaterialPass(MaterialShader shader, int renderFlags, Bindable[] bindables) {
        this.shader = shader;
        this.renderFlags = renderFlags;
        this.bindables = bindables;
    }

    public MaterialShader getShader() {
        return shader;
    }

    public Bindable[] getBindables() {
        return bindables;
    }

    public int getRenderFlags() {
        return renderFlags;
    }
}
