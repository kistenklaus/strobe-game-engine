package org.strobe.gfx;

import org.strobe.debug.Debug;
import org.strobe.debug.Debuggable;
import org.strobe.window.Window;

public final class Graphics implements Debuggable {

    @Debug
    private final MasterRenderer masterRenderer = new MasterRenderer();
    @Debug
    private final BindingCodex codex = new BindingCodex();
    private final Window window;

    public Graphics(Window window) {
        this.window = window;
    }

    public void bind(Bindable bindable) {
        if (bindable == null) throw new IllegalArgumentException("can't bind null object");
        bindable.pool.bind(this, bindable);
    }

    public void unbind(Bindable bindable) {
        bindable.pool.unbind(this, bindable);
    }

    public void dispose() {
        codex.dispose(this);
        masterRenderer.dispose(this);
        window.close();
        window.dispose();
    }

    public void dispose(Bindable bindable) {
        bindable.pool.dispose(this, bindable);
    }

    public <T extends Renderer> T addRenderer(int renderPrio, T renderer) {
        masterRenderer.add(renderPrio, renderer);
        return renderer;
    }

    public void removeRenderer(Renderer renderer) {
        masterRenderer.remove(renderer);
    }

    public void render() {
        masterRenderer.render(this);
    }

    public boolean shouldClose() {
        return window.shouldClose();
    }

    public void newFrame(){
        window.newFrame();
    }

    public void endFrame(){
        window.endFrame();
    }

    public void pollEvents() {
        window.pollEvents();
    }

    public void swapBuffers() {
        window.swapBuffers();
    }

    public <P extends BindablePool> P getPool(Class<P> poolClass) {
        return codex.getPool(poolClass);
    }


    public void init() {
        window.create();
    }

    public Window getWindow() {
        return window;
    }

    public int getWidth() {
        return window.getWidth();
    }

    public int getHeight() {
        return window.getHeight();
    }
}