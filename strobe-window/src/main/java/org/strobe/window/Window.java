package org.strobe.window;

public abstract class Window {

    private String title;
    private int width;
    private int height;

    protected WindowMouse mouse = new WindowMouse();
    protected WindowKeyboard keyboard = new WindowKeyboard();
    protected WindowEventHandler windowEventHandler = new WindowEventHandler();

    protected WindowContentRegion contentRegion = null;

    public Window(String title, int width, int height) {
        this.title = title;
        this.width = width;
        this.height = height;
        windowEventHandler.addWindowEventListener(new WindowEventListener() {
            @Override
            public void onResize(int width, int height) {
                mouse.setWindowSize(width, height);
            }

            @Override
            public void onFocusChange(boolean focus) {
                //stub
            }

            @Override
            public void onPositionChange(int xpos, int ypos) {
                //stub
            }
        });
    }

    public abstract void create();

    public void newFrame(){
        pollEvents();
    }

    public void endFrame(){
        swapBuffers();
    }

    public void pollEvents(){
        mouse.beforePoll();
        poolEventsImpl();
        mouse.afterPoll();
    }

    protected abstract void poolEventsImpl();

    public abstract void swapBuffers();

    public abstract boolean shouldClose();

    public abstract void close();

    public abstract void dispose();

    public abstract long pointer();

    public int getWidth(){
        return this.width;
    }

    public int getHeight(){
        return this.height;
    }

    public String getTitle() {
        return title;
    }

    protected void setWidth(int width) {
        this.width = width;
    }

    protected void setHeight(int height) {
        this.height = height;
    }

    public WindowEventHandler getWindowEventHandler(){
        return windowEventHandler;
    }

    public WindowMouse getMouse() {
        return mouse;
    }

    public WindowKeyboard getKeyboard() {
        return keyboard;
    }

    public void setContentRegion(WindowContentRegion contentRegion){
        this.contentRegion = contentRegion;
        mouse.setContentRegion(contentRegion);
        keyboard.setContentRegion(contentRegion);
    }

    public WindowContentRegion getContentRegion(){
        return contentRegion;
    }
}
