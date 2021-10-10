package org.strobe.engine;

import org.strobe.gfx.Graphics;
import org.strobe.window.Window;


public abstract class StrobeEngine<T extends StrobeContext> implements Runnable {

    private final Thread thread;
    protected final T context;
    private final Graphics gfx;
    private boolean running = false;

    public StrobeEngine(T context, Window window) {
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

    public void beforeInit(Graphics gfx) {
    }

    public void afterInit(Graphics gfx) {
    }

    public void run() {
        gfx.init();
        beforeInit(gfx);
        context.init(gfx);
        afterInit(gfx);

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
        gfx.newFrame();
        context.render(gfx);
        //render
        beforeRender(gfx);
        gfx.render();
        afterRender(gfx);
        //context render (submit)
        //render debug
        //logic
        beforeUpdate(dt);
        context.update(dt);
        afterUpdate(dt);
        //swapping buffers (syncing of gpu and cpu)
        beforeSwapBuffers(gfx);
        gfx.endFrame();
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
