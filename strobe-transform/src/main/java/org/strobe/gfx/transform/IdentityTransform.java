package org.strobe.gfx.transform;

public final class IdentityTransform extends AbstractTransform {

    public IdentityTransform(AbstractTransform parent) {
        super(parent, null, null, null);
    }

    public IdentityTransform(){
        super(getRoot(), null,null,null);
    }

    @Override
    public void updateTransformationMatrix() {
        transformationMatrix.identity();
    }
}
