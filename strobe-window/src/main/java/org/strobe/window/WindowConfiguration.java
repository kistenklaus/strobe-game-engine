package org.strobe.window;

import javax.swing.*;

public final class WindowConfiguration {

    private static final boolean DEFAULT_NATIVE_FULLSCREEN = false;
    private static final boolean DEFAULT_BORDERLESS_FULLSCREEN = false;
    private static final boolean DEFAULT_MAXIMIZE = false;
    private static final boolean DEFAULT_RESIZABLE = false;
    private static final boolean DEFAULT_VSYNC = false;

    public static WindowConfiguration get(){
        return new WindowConfiguration(DEFAULT_NATIVE_FULLSCREEN, DEFAULT_BORDERLESS_FULLSCREEN, DEFAULT_MAXIMIZE, DEFAULT_RESIZABLE, DEFAULT_VSYNC);
    }

    public static WindowConfiguration get(boolean nativeFullscreen, boolean borderlessFullscreen, boolean maximize, boolean resizable, boolean vsync){
        return new WindowConfiguration(nativeFullscreen, borderlessFullscreen, maximize, resizable, vsync);
    }

    private boolean nativeFullscreen;
    private boolean borderlessFullscreen;
    private boolean maximized;
    private boolean resizable;
    private boolean vsync;

    public WindowConfiguration(boolean nativeFullscreen, boolean borderlessFullscreen, boolean maximized, boolean resizable, boolean vsync) {
        this.nativeFullscreen = nativeFullscreen;
        this.borderlessFullscreen = borderlessFullscreen;
        this.maximized = maximized;
        this.resizable = resizable;
        this.vsync = vsync;
    }

    public WindowConfiguration enableNativeFullscreen() {
        nativeFullscreen = true;
        return this;
    }

    public WindowConfiguration disableNativeFullscreen(){
        nativeFullscreen = false;
        return this;
    }

    public WindowConfiguration enableBorderlessFullscreen(){
        borderlessFullscreen = true;
        return this;
    }

    public WindowConfiguration disableBorderlessFullscreen(){
        borderlessFullscreen = false;
        return this;
    }

    public WindowConfiguration enableMaximize(){
        maximized = true;
        return this;
    }

    public WindowConfiguration disableMaximize(){
        maximized = false;
        return this;
    }

    public WindowConfiguration enableResizing(){
        resizable = true;
        return this;
    }

    public WindowConfiguration disableResizable(){
        resizable = false;
        return this;
    }

    public WindowConfiguration enableVSync(){
        vsync = true;
        return this;
    }

    public WindowConfiguration disableVSync(){
        vsync = true;
        return this;
    }

    public boolean isResizableEnabled(){
        return resizable;
    }

    public boolean isNativeFullscreenEnabled(){
        return nativeFullscreen;
    }

    public boolean isBorderlessFullscreenEnabled(){
        return borderlessFullscreen;
    }

    public boolean isVSyncEnabled(){
        return vsync;
    }

    public boolean isMaximizeEnabled(){
        return maximized;
    }




}
