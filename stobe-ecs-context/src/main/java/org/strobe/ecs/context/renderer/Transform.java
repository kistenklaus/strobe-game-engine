package org.strobe.ecs.context.renderer;

import org.joml.Quaternionf;
import org.joml.Vector3f;
import org.strobe.ecs.Component;
import org.strobe.gfx.transform.AbstractTransform;

public class Transform extends AbstractTransform implements Component {



    public Transform(Vector3f position, Vector3f scale, Quaternionf orientation) {
        super(null, position, scale, orientation);
    }

    public Transform(){
        this(null,null,null);
    }

    @Override
    public void updateTransformationMatrix() {
        transformationMatrix.identity();
        transformationMatrix.translate(position);
        transformationMatrix.rotate(orientation);
        transformationMatrix.scale(scale);
    }
}
