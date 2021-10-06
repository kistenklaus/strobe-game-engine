package org.strobe.engine;

import org.strobe.gfx.Graphics;
import org.strobe.window.Window;


public abstract class StrobeEngine implements Runnable {

    private final Thread thread;
    private final StrobeContext context;
    private final Graphics gfx;
    private boolean running = false;

    public StrobeEngine(StrobeContext context, Window window) {
        context.linkToEngine(this);
        this.context = context;
        this.thread = new Thread(this);
        this.gfx = new Graphics(window);
    }

    void start() {
        running = true;
        thread.start();
    }

    void stop() {
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
        while (running && !gfx.shouldClose()) {
            long curr = System.nanoTime();
            float dt = (curr - last) * 1e-9f;
            last = curr;
            loop(dt);
        }
        running = false;
        gfx.dispose();
    }

    private void loop(float dt) {
        context.render(gfx);
        //render
        beforeRender(gfx);
        gfx.render();
        afterRender(gfx);
        //context render (submit)
        //render debug
        //logic
        gfx.pollEvents();
        beforeUpdate(dt);
        context.update(dt);
        afterUpdate(dt);
        //swapping buffers (syncing of gpu and cpu)
        beforeSwapBuffers(gfx);
        gfx.swapBuffers();
        afterSwapBuffers(gfx);
    }

    protected void beforeRender(Graphics gfx) {
    }

    protected void afterRender(Graphics gfx) {
    }

    protected void beforeUpdate(float dt) {
    }

    protected void afterUpdate(float dt) {
    }

    protected void beforeSwapBuffers(Graphics gfx) {
    }

    protected void afterSwapBuffers(Graphics gfx) {
    }
}
