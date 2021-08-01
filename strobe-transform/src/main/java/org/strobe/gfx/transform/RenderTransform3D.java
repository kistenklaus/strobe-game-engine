package org.strobe.gfx.transform;

import org.joml.Quaternionf;
import org.joml.Vector3f;

public class RenderTransform3D extends AbstractTransform {


    public RenderTransform3D(){
        super(null,null,null,null);
    }
    public RenderTransform3D(AbstractTransform parent, Vector3f position, Vector3f scale, Quaternionf orientation) {
        super(parent, position, scale, orientation);
    }

    @Override
    public void updateTransformationMatrix() {
        transformationMatrix.identity();
        transformationMatrix.translate(position);
        transformationMatrix.rotate(orientation);
        transformationMatrix.scale(scale);
    }
}
