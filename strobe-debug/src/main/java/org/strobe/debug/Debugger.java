package org.strobe.debug;

public abstract class Debugger {

    public abstract void debug(Debuggable debuggable);

    public abstract void render();

    public abstract void dispose();
}
