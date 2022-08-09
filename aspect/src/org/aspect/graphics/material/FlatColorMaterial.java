package org.aspect.graphics.material;

import org.aspect.Aspect;
import org.aspect.graphics.Graphics;
import org.joml.Vector3f;

public class FlatColorMaterial extends Material implements ForwardMaterial {

    private static final String VERTEX_PATH = Graphics.SHADERS_LOCATION + "flatColor.vs";
    private static final String FRAGMENT_PATH = Graphics.SHADERS_LOCATION + "flatColor.fs";

    public FlatColorMaterial(Vector3f color) {
        super(new MaterialAttachment(Aspect.loadShaderFromClasspath(VERTEX_PATH, FRAGMENT_PATH), 3, "vec3 color"));
        getFMA().setPropertyVec3("color", color);
    }

    @Override
    public MaterialAttachment getFMA() {
        return attachments[0];
    }
}
