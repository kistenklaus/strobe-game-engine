package org.aspect.graphics.window.event;

import org.aspect.graphics.window.event.WindowEvent;

public final class WindowMouseEvent extends WindowEvent {



    private final int button;
    private final int mods;

    public WindowMouseEvent(int button, int mods){
        this.button = button;
        this.mods = mods;
    }

    public int getButton(){
        return button;
    }

    public int getMods(){
        return mods;
    }

    public boolean hasMod(int mod){
        return (mod & this.mods) != 0;
    }
}
