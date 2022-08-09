package org.aspect.graphics.material;

import org.aspect.Aspect;
import org.aspect.graphics.Graphics;
import org.aspect.graphics.bindable.Texture;
import org.aspect.graphics.renderer.CullFace;

public class LambertianTexMaterial extends Material implements ForwardMaterial, DeferredMaterial {

    private static final String FORWARD_VERTEX_PATH = Graphics.SHADERS_LOCATION + "lambertianTexture.vs";
    private static final String FORWARD_FRAGMENT_PATH = Graphics.SHADERS_LOCATION + "lambertianTexture.fs";
    private static final String DEFERRED_VERTEX_PATH = Graphics.SHADERS_LOCATION + "deferredTexture.vs";
    private static final String DEFERRED_FRAGMENT_PATH = Graphics.SHADERS_LOCATION + "deferredTexture.fs";

    public LambertianTexMaterial(String diffusePath, String specularPath) {
        super(
                new MaterialAttachment(Aspect.loadShaderFromClasspath(FORWARD_VERTEX_PATH, FORWARD_FRAGMENT_PATH)),
                new MaterialAttachment(Aspect.loadShaderFromClasspath(DEFERRED_VERTEX_PATH, DEFERRED_FRAGMENT_PATH)));
        Texture diffuse = Aspect.loadTexture(diffusePath, true);
        Texture specular = Aspect.loadTexture(specularPath, true);

        getFMA().addTexture("diffuseMap", diffuse);
        getFMA().addTexture("specularMap", specular);
        getFMA().setCullFace(CullFace.NONE);

        getDMA().addTexture("diffuseMap", diffuse);
        getDMA().addTexture("specularMap", specular);
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
