package org.aspect.window.event;

public final class WindowMouseButtonEvent extends WindowEvent {


    private final int button;
    private final int mods;

    public WindowMouseButtonEvent(int button, int mods){
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
