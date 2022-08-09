package org.aspect.graphics.material;

import org.aspect.Aspect;
import org.aspect.graphics.Graphics;
import org.aspect.graphics.bindable.Shader;
import org.joml.Vector4f;

public class LambertianColorMaterial extends Material implements DeferredMaterial, ForwardMaterial {

    private static final String FORWARD_VERTEX_PATH = Graphics.SHADERS_LOCATION + "lambertianColor.vs";
    private static final String FORWARD_FRAGMENT_PATH = Graphics.SHADERS_LOCATION + "lambertianColor.fs";
    private static final String DEFERRED_VERTEX_PATH = Graphics.SHADERS_LOCATION + "deferredColor.vs";
    private static final String DEFERRED_FRAGMENT_PATH = Graphics.SHADERS_LOCATION + "deferredColor.fs";

    public LambertianColorMaterial(Vector4f color, float specular) {
        super(new Shader[]{
                Aspect.loadShaderFromClasspath(FORWARD_VERTEX_PATH, FORWARD_FRAGMENT_PATH),
                Aspect.loadShaderFromClasspath(DEFERRED_VERTEX_PATH, DEFERRED_FRAGMENT_PATH)
        }, 3, "vec4 color", "float specular");
        getSharedUbo().uniformVec4("color", new Vector4f(color));
        getSharedUbo().uniform1f("specular", specular);
    }

    @Override
    public MaterialAttachment getFMA() {
        return attachments[0];
    }

    @Override
    public MaterialAttachment getDMA() {
        return attachments[1];
    }
}
