package org.strobe.gfx.materials;

import org.strobe.gfx.lights.LightConstants;
import org.strobe.gfx.opengl.bindables.shader.ShaderCompileConstantSet;

public final class MaterialShaderCompileConstantSet extends ShaderCompileConstantSet {

    private static MaterialShaderCompileConstantSet instance = null;

    public static MaterialShaderCompileConstantSet getInstance(){
        if(instance == null)return new MaterialShaderCompileConstantSet();
        return instance;
    }

    private MaterialShaderCompileConstantSet(){
        super(
                "DIRECTIONAL_LIGHT_COUNT = " + LightConstants.DIRECTIONAL_LIGHT_COUNT,
                "MAX_CASTING_DIR_LIGHTS = " + LightConstants.MAX_DIR_CASTING_LIGHTS
        );
    }


}
