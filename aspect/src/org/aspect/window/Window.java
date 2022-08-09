package org.aspect.window;

import org.aspect.graphics.Graphics;
import org.aspect.graphics.renderer.Renderer;
import org.aspect.window.listener.WindowKeyListener;
import org.aspect.window.listener.WindowListener;
import org.aspect.window.listener.WindowMouseListener;

public abstract class Window {

    private int width, height;
    private String title;
    private boolean fullscreen;
    private Renderer renderer;
    private Graphics graphics;

    private WindowKeyListener keyListener = null;
    private WindowMouseListener mouseListener = null;
    private WindowListener windowListener = null;

    public Window() {
    }

    public void initializeWindow(Graphics graphics, String title,
                                 int width, int height, boolean fullscreen) {
        this.graphics = graphics;
        this.title = title;
        this.width = width;
        this.height = height;
        this.fullscreen = fullscreen;
        createWindow();
        this.renderer = this.graphics.createRenderer(this);
    }

    protected abstract void createWindow();

    public final void pollEvents(){
        if(keyListener != null)keyListener.onPoll();
        if(mouseListener != null)mouseListener.onPoll();
        pollWindowEvents();
    }

    public final void update(){
        renderer.render();
        swapBuffers();
    }

    protected abstract void pollWindowEvents();

    public abstract void swapBuffers();

    public abstract void close();

    public abstract void dispose();

    public abstract boolean shouldClose();


    public <T extends WindowKeyListener> T setWindowKeyListener(T keyListener){
        connectWindowKeyListener(keyListener);
        this.keyListener = keyListener;
        return keyListener;
    }

    protected abstract void connectWindowKeyListener(WindowKeyListener keyListener);

    public <T extends WindowMouseListener> T setWindowMouseListener(T mouseListener){
        connectWindowMouseListener(mouseListener);
        this.mouseListener = mouseListener;
        return mouseListener;
    }

    protected abstract void connectWindowMouseListener(WindowMouseListener mouseListener);


    public <T extends WindowListener> T setWindowListener(T windowListener){
        connectWindowListener(windowListener);
        this.windowListener = windowListener;
        return windowListener;
    }

    protected abstract void connectWindowListener(WindowListener windowListener);

    public Renderer getRenderer() {
        return renderer;
    }

    public String getTitle() {
        return title;
    }

    public int getWidth() {
        return width;
    }

    public void setWidth(int width) {
        this.width = width;
    }

    public int getHeight() {
        return height;
    }

    public void setHeight(int height) {
        this.height = height;
    }

    public boolean isFullscreen() {
        return fullscreen;
    }
}
