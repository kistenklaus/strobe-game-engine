package org.aspect.graphics.data;

import org.aspect.graphics.bindable.Bindable;
import org.aspect.graphics.bindable.UniformBuffer;

import java.util.Objects;

public class CameraBuffer {

    private final UniformBuffer ubo;

    private final Camera camera;

    public CameraBuffer(Camera camera, UniformBuffer ubo) {
        this.ubo = ubo;
        this.camera = camera;
    }

    public UniformBuffer getUbo() {
        return ubo;
    }

    public Camera getCamera() {
        return camera;
    }
}
