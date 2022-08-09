package org.aspect.window.event;

public final class WindowKeyEvent extends WindowEvent {

    private final int key;
    private final int mods;

    public WindowKeyEvent(int key, int mods) {
        this.key = key;
        this.mods = mods;
    }

    public int getKey() {
        return key;
    }

    public int getMods() {
        return mods;
    }

    public boolean hasMod(int mod) {
        return (mod & this.mods) != 0;
    }

}
