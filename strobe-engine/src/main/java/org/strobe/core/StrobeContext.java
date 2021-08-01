package org.strobe.core;

import org.strobe.debug.Debugger;
import org.strobe.gfx.Graphics;

public abstract class StrobeContext {

    private StrobeEngine engine;

    public abstract void init(Graphics gfx);

    public abstract void render(Graphics gfx);

    public abstract void update(float dt);

    public void addDebugger(Debugger debugger){
        engine.addDebugger(debugger);
    }

    public void start() {
        engine.start();
    }

    public void stop() {
        engine.stop();
    }

    final void linkToEngine(StrobeEngine engine) {
        this.engine = engine;
    }

}
