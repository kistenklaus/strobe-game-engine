package org.strobe.gfx.camera;

import org.joml.Matrix4f;
import org.joml.Vector3f;

public record FrustumBox(Vector3f nearBottomLeft, Vector3f nearTopLeft,
                         Vector3f nearTopRight, Vector3f nearBottomRight,
                         Vector3f farBottomLeft, Vector3f farTopLeft,
                         Vector3f farTopRight, Vector3f farBottomRight) {

    public static FrustumBox getFromProjView(Matrix4f projView) {
        return new FrustumBox(
                projView.frustumCorner(Matrix4f.CORNER_NXNYNZ, new Vector3f()),
                projView.frustumCorner(Matrix4f.CORNER_NXPYNZ, new Vector3f()),
                projView.frustumCorner(Matrix4f.CORNER_PXPYNZ, new Vector3f()),
                projView.frustumCorner(Matrix4f.CORNER_PXNYNZ, new Vector3f()),
                projView.frustumCorner(Matrix4f.CORNER_NXNYPZ, new Vector3f()),
                projView.frustumCorner(Matrix4f.CORNER_NXPYPZ, new Vector3f()),
                projView.frustumCorner(Matrix4f.CORNER_PXPYPZ, new Vector3f()),
                projView.frustumCorner(Matrix4f.CORNER_PXNYPZ, new Vector3f()));
    }

    public float[] toFloatArray_vec3aligned() {
        return new float[]{
                this.nearBottomLeft().x, this.nearBottomLeft().y, this.nearBottomLeft().z,
                this.nearTopLeft().x, this.nearTopLeft().y, this.nearTopLeft().z,
                this.nearTopRight().x, this.nearTopRight().y, this.nearTopRight().z,
                this.nearBottomRight().x, this.nearBottomRight().y, this.nearBottomRight().z,

                this.farBottomLeft().x, this.farBottomLeft().y, this.farBottomLeft().z,
                this.farTopLeft().x, this.farTopLeft().y, this.farTopLeft().z,
                this.farTopRight().x, this.farTopRight().y, this.farTopRight().z,
                this.farBottomRight().x, this.farBottomRight().y, this.farBottomRight().z
        };
    }

    public FrustumBox transform(Matrix4f mat4){
        return new FrustumBox(
                mat4.transformPosition(nearBottomLeft, new Vector3f()),
                mat4.transformPosition(nearTopLeft, new Vector3f()),
                mat4.transformPosition(nearTopRight, new Vector3f()),
                mat4.transformPosition(nearBottomRight, new Vector3f()),
                mat4.transformPosition(farBottomLeft, new Vector3f()),
                mat4.transformPosition(farTopLeft, new Vector3f()),
                mat4.transformPosition(farTopRight, new Vector3f()),
                mat4.transformPosition(farBottomRight, new Vector3f())
        );
    }

    public Vector3f calculateCenter(){
        Vector3f nearCenter = nearBottomLeft.add(nearTopRight, new Vector3f()).mul(0.5f);
        Vector3f farCenter = farBottomLeft.add(farTopRight, new Vector3f()).mul(0.5f);
        return nearCenter.add(farCenter).mul(0.5f);
    }
}
