package org.strobe.gfx.camera;

import org.joml.Matrix4f;
import org.joml.Vector3f;
import org.strobe.gfx.Graphics;
import org.strobe.gfx.transform.AbstractTransform;

import java.util.function.Supplier;

public final class LenseCamera extends AbstractCamera{

    private final Supplier<Lense> lense;
    private final Supplier<AbstractTransform> transform;

    public LenseCamera(Graphics gfx, int horResolution, int verResolution,
                       Supplier<Lense> lense,
                       Supplier<AbstractTransform> transform) {
        super(gfx, horResolution, verResolution);
        this.lense = lense;
        this.transform = transform;
    }

    @Override
    public void update() {
        viewMatrix.identity();
        AbstractTransform transform = this.transform.get();
        if (transform != null) {
            position.set(transform.getPosition());
            transform.getTransformationMatrix().invert(viewMatrix);
        }else position.set(0,0,0);

        //TODO conditional proj update
        Lense lense = this.lense.get();
        projMatrix.identity();
        if(lense != null){
            lense.project(projMatrix);
        }
    }

    @Override
    public float getNear() {
        if(lense.get() == null)return 0;
        return lense.get().getNear();
    }

    @Override
    public float getFar() {
        if(lense.get() == null)return 1;
        return lense.get().getFar();
    }

    @Override
    public FrustumBox getFrustumBox() {
        return FrustumBox.getFromProjView(projMatrix.mul(viewMatrix, new Matrix4f()));
    }
}
