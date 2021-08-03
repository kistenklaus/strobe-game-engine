package org.strobe.ecs.context.renderer.transform;

import org.joml.Matrix4f;
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

    public Transform(Vector3f position){
        this(position, null,null);
    }

    @Override
    public void updateTransformationMatrix() {
        transformationMatrix.identity();
        transformationMatrix.translate(position);
        Matrix4f rot = orientation.get(new Matrix4f());
        transformationMatrix.mul(rot);
        transformationMatrix.scale(scale);
    }
}
