package org.strobe.core;

import org.strobe.debug.Debuggable;
import org.strobe.debug.Debugger;
import org.strobe.debug.MasterDebugger;
import org.strobe.gfx.Graphics;
import org.strobe.window.Window;


class StrobeEngine implements Runnable{

    private final Thread thread;
    private final StrobeContext context;
    private final Graphics gfx;
    private final MasterDebugger masterDebugger = new MasterDebugger();
    private boolean running = false;

    StrobeEngine(StrobeContext context, Window window){
        context.linkToEngine(this);
        this.context = context;
        this.thread = new Thread(this);
        this.gfx = new Graphics(window);
    }

    void start(){
        running = true;
        thread.start();
    }

    void stop(){
        running = false;
        try {
            thread.join();
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    public void run() {
        gfx.init();
        context.init(gfx);

        long last = System.nanoTime();
        while(running && !gfx.shouldClose()){
            long curr = System.nanoTime();
            float dt = (curr - last) * 1e-9f;
            last = curr;
            loop(dt);
        }
        running = false;
        gfx.dispose();
        masterDebugger.dispose();
    }

    private void loop(float dt){
        context.render(gfx);
        masterDebugger.debug(gfx);
        //render
        gfx.render();
        masterDebugger.render();
        //context render (submit)
        //render debug
        //logic
        gfx.pollEvents();
        context.update(dt);
        //swapping buffers (syncing of gpu and cpu)
        gfx.swapBuffers();
    }

    public void debug(Debuggable debuggable){
        masterDebugger.debug(debuggable);
    }

    public void addDebugger(Debugger debugger){
        masterDebugger.addDebugger(debugger);
    }
}
