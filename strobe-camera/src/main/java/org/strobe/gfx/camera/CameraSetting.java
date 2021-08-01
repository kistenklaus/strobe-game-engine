package org.strobe.gfx.camera;

public final class CameraSetting {

    public static final int LINEAR_INTERPOLATION = 0;
    public static final int FXAA = LINEAR_INTERPOLATION+1;
    public static final int SETTINGS_LAST = FXAA;


    private boolean[] settings = new boolean[SETTINGS_LAST+1];

    public void enable(int setting){
        settings[setting] = true;
    }

    public void disable(int setting){
        settings[setting] = false;
    }

    public boolean isEnabled(int setting){
        return settings[setting];
    }
}
