package org.strobe.gfx.camera;

import org.strobe.gfx.Graphics;
import org.strobe.gfx.Pool;
import org.strobe.gfx.opengl.bindables.ubo.Ubo;
import org.strobe.gfx.opengl.bindables.ubo.UboPool;

import java.util.Arrays;

@Pool(UboPool.class)
public final class CameraUbo extends Ubo {

    public CameraUbo(Graphics gfx) {
        super(gfx, "cameras", 0, true,
                "mat4 view",
                "mat4 proj",
                "vec3 cameraPosition");
    }

    public void uniformCamera(Graphics gfx, AbstractCamera camera) {
        super.uniform(gfx, "view", camera.getViewMatrix());
        super.uniform(gfx, "proj", camera.getProjMatrix());
        super.uniform(gfx, "cameraPosition", camera.getPosition());
    }
}
