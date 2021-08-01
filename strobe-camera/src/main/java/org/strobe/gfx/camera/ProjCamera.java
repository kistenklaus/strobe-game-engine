package org.strobe.gfx.camera;

public class ProjCamera extends AbstractCamera{

    public ProjCamera(int xres, int yres, float fov, float near, float far){
        super(xres, yres);
        projMatrix.perspective((float) Math.toRadians(fov), xres/(float)yres, near, far);
    }

}
