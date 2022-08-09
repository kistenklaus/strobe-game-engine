package org.aspect.graphics.passes.queues;

import org.aspect.graphics.bindable.setter.Blender;
import org.aspect.graphics.bindable.setter.DepthTester;
import org.aspect.graphics.drawable.Transform;
import org.joml.Vector3f;

public class ZSortedSceneQueuePass extends SceneQueuePass {

    public static final String TARGET = SceneQueuePass.TARGET;
    public static final String CAMERA = SceneQueuePass.CAMERA;
    public static final String LIGHT_UBO = SceneQueuePass.LIGHT_UBO;

    public ZSortedSceneQueuePass(String name) {
        super(name);
    }

    public void completeSettings() {
        addBindable(new Blender());
        addBindable(new DepthTester());
    }

    public void prepare() {
        sortDrawable((d1, d2) -> {
            Transform t1 = d1.getTransform();
            Transform t2 = d2.getTransform();
            Vector3f cp = camera.get().getCamera().getPosition();
            return -Float.compare(Vector3f.length(t1.getPosX() - cp.x, t1.getPosY() - cp.y, t1.getPosZ() - cp.z),
                    Vector3f.length(t2.getPosX() - cp.x, t2.getPosY() - cp.y, t2.getPosZ() - cp.z));
        });
    }
}
