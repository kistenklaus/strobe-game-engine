package org.strobe.gfx.materials;

import org.strobe.gfx.Bindable;
import org.strobe.gfx.materials.shaders.MaterialShader;

public record MaterialPass(MaterialShader shader, int flags,
                           Bindable...bindables) {

    public static final int FORWARD_PASS = 1;
    public static final int SHADOW_MAP_PASS = 2;

}
