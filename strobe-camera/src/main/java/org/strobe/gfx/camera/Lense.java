package org.strobe.gfx.camera;

import org.joml.Matrix4f;

public interface Lense {

    void project(Matrix4f projectionMatrix);

    float getNear();
    float getFar();

}
