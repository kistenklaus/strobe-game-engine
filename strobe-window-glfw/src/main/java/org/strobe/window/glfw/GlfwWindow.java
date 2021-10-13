package org.strobe.window.glfw;

import org.lwjgl.glfw.GLFWVidMode;
import org.lwjgl.opengl.GL;
import org.lwjgl.system.MemoryStack;
import org.strobe.window.*;

import java.nio.IntBuffer;
import java.util.Objects;

import static org.lwjgl.glfw.GLFW.*;
import static org.lwjgl.system.MemoryStack.stackGet;
import static org.lwjgl.system.MemoryStack.stackPush;

public class GlfwWindow extends Window {

    protected static final WindowButton[] BUTTON_MAPPING = new WindowButton[GLFW_MOUSE_BUTTON_LAST+1];

    protected static final WindowKey[] KEY_MAPPING = new WindowKey[GLFW_KEY_LAST+1];

    static{
        BUTTON_MAPPING[GLFW_MOUSE_BUTTON_RIGHT] = WindowButton.RIGHT;
        BUTTON_MAPPING[GLFW_MOUSE_BUTTON_LEFT] = WindowButton.LEFT;
        BUTTON_MAPPING[GLFW_MOUSE_BUTTON_MIDDLE] = WindowButton.MIDDLE;

        KEY_MAPPING[GLFW_KEY_SPACE] = WindowKey.KEY_SPACE;
        KEY_MAPPING[GLFW_KEY_APOSTROPHE] = WindowKey.KEY_APOSTROPHE;
        KEY_MAPPING[GLFW_KEY_COMMA] = WindowKey.KEY_COMMA;
        KEY_MAPPING[GLFW_KEY_MINUS] = WindowKey.KEY_MINUS;
        KEY_MAPPING[GLFW_KEY_PERIOD] = WindowKey.KEY_PERIOD;
        KEY_MAPPING[GLFW_KEY_SLASH] = WindowKey.KEY_SLASH;

        KEY_MAPPING[GLFW_KEY_SEMICOLON] = WindowKey.KEY_SEMICOLON;
        KEY_MAPPING[GLFW_KEY_EQUAL] = WindowKey.KEY_EQUAL;

        KEY_MAPPING[GLFW_KEY_0] = WindowKey.KEY_0;
        KEY_MAPPING[GLFW_KEY_1] = WindowKey.KEY_1;
        KEY_MAPPING[GLFW_KEY_2] = WindowKey.KEY_2;
        KEY_MAPPING[GLFW_KEY_3] = WindowKey.KEY_3;
        KEY_MAPPING[GLFW_KEY_4] = WindowKey.KEY_4;
        KEY_MAPPING[GLFW_KEY_5] = WindowKey.KEY_5;
        KEY_MAPPING[GLFW_KEY_6] = WindowKey.KEY_6;
        KEY_MAPPING[GLFW_KEY_7] = WindowKey.KEY_7;
        KEY_MAPPING[GLFW_KEY_8] = WindowKey.KEY_8;
        KEY_MAPPING[GLFW_KEY_9] = WindowKey.KEY_9;

        KEY_MAPPING[GLFW_KEY_A] = WindowKey.KEY_A;
        KEY_MAPPING[GLFW_KEY_B] = WindowKey.KEY_B;
        KEY_MAPPING[GLFW_KEY_C] = WindowKey.KEY_C;
        KEY_MAPPING[GLFW_KEY_D] = WindowKey.KEY_D;
        KEY_MAPPING[GLFW_KEY_E] = WindowKey.KEY_E;
        KEY_MAPPING[GLFW_KEY_F] = WindowKey.KEY_F;
        KEY_MAPPING[GLFW_KEY_G] = WindowKey.KEY_G;
        KEY_MAPPING[GLFW_KEY_H] = WindowKey.KEY_H;
        KEY_MAPPING[GLFW_KEY_I] = WindowKey.KEY_I;
        KEY_MAPPING[GLFW_KEY_J] = WindowKey.KEY_J;
        KEY_MAPPING[GLFW_KEY_K] = WindowKey.KEY_K;
        KEY_MAPPING[GLFW_KEY_L] = WindowKey.KEY_L;
        KEY_MAPPING[GLFW_KEY_M] = WindowKey.KEY_M;
        KEY_MAPPING[GLFW_KEY_N] = WindowKey.KEY_N;
        KEY_MAPPING[GLFW_KEY_O] = WindowKey.KEY_O;
        KEY_MAPPING[GLFW_KEY_P] = WindowKey.KEY_P;
        KEY_MAPPING[GLFW_KEY_Q] = WindowKey.KEY_Q;
        KEY_MAPPING[GLFW_KEY_R] = WindowKey.KEY_R;
        KEY_MAPPING[GLFW_KEY_S] = WindowKey.KEY_S;
        KEY_MAPPING[GLFW_KEY_T] = WindowKey.KEY_T;
        KEY_MAPPING[GLFW_KEY_U] = WindowKey.KEY_U;
        KEY_MAPPING[GLFW_KEY_V] = WindowKey.KEY_V;
        KEY_MAPPING[GLFW_KEY_W] = WindowKey.KEY_W;
        KEY_MAPPING[GLFW_KEY_X] = WindowKey.KEY_X;
        KEY_MAPPING[GLFW_KEY_Y] = WindowKey.KEY_Y;
        KEY_MAPPING[GLFW_KEY_Z] = WindowKey.KEY_Z;

        KEY_MAPPING[GLFW_KEY_ESCAPE] = WindowKey.KEY_ESCAPE;
        KEY_MAPPING[GLFW_KEY_ENTER] = WindowKey.KEY_ENTER;
        KEY_MAPPING[GLFW_KEY_TAB] = WindowKey.KEY_TAB;
        KEY_MAPPING[GLFW_KEY_BACKSPACE] = WindowKey.KEY_BACKSPACE;
        KEY_MAPPING[GLFW_KEY_INSERT] = WindowKey.KEY_INSERT;
        KEY_MAPPING[GLFW_KEY_DELETE] = WindowKey.KEY_DELETE;
        KEY_MAPPING[GLFW_KEY_RIGHT] = WindowKey.KEY_RIGHT;
        KEY_MAPPING[GLFW_KEY_LEFT] = WindowKey.KEY_LEFT;
        KEY_MAPPING[GLFW_KEY_DOWN] = WindowKey.KEY_DOWN;
        KEY_MAPPING[GLFW_KEY_UP] = WindowKey.KEY_UP;
        KEY_MAPPING[GLFW_KEY_PAGE_UP] = WindowKey.KEY_PAGE_UP;
        KEY_MAPPING[GLFW_KEY_PAGE_DOWN] = WindowKey.KEY_PAGE_DOWN;
        KEY_MAPPING[GLFW_KEY_HOME] = WindowKey.KEY_HOME;
        KEY_MAPPING[GLFW_KEY_END] = WindowKey.KEY_END;
        KEY_MAPPING[GLFW_KEY_CAPS_LOCK] = WindowKey.KEY_CAPS_LOCK;
        KEY_MAPPING[GLFW_KEY_KP_0] = WindowKey.KEY_KP_0;
        KEY_MAPPING[GLFW_KEY_KP_1] = WindowKey.KEY_KP_1;
        KEY_MAPPING[GLFW_KEY_KP_2] = WindowKey.KEY_KP_2;
        KEY_MAPPING[GLFW_KEY_KP_3] = WindowKey.KEY_KP_3;
        KEY_MAPPING[GLFW_KEY_KP_4] = WindowKey.KEY_KP_4;
        KEY_MAPPING[GLFW_KEY_KP_5] = WindowKey.KEY_KP_5;
        KEY_MAPPING[GLFW_KEY_KP_6] = WindowKey.KEY_KP_6;
        KEY_MAPPING[GLFW_KEY_KP_7] = WindowKey.KEY_KP_7;
        KEY_MAPPING[GLFW_KEY_KP_8] = WindowKey.KEY_KP_8;
        KEY_MAPPING[GLFW_KEY_KP_9] = WindowKey.KEY_KP_9;

        KEY_MAPPING[GLFW_KEY_LEFT_SHIFT] = WindowKey.KEY_LEFT_SHIFT;
        KEY_MAPPING[GLFW_KEY_RIGHT_SHIFT] = WindowKey.KEY_RIGHT_SHIFT;
        KEY_MAPPING[GLFW_KEY_LEFT_ALT] = WindowKey.KEY_LEFT_ALT;
        KEY_MAPPING[GLFW_KEY_RIGHT_ALT] = WindowKey.KEY_RIGHT_ALT;
        KEY_MAPPING[GLFW_KEY_LEFT_CONTROL] = WindowKey.KEY_LEFT_CTRL;
        KEY_MAPPING[GLFW_KEY_RIGHT_CONTROL] = WindowKey.KEY_RIGHT_CTRL;
        KEY_MAPPING[GLFW_KEY_LEFT_SUPER] = WindowKey.KEY_LEFT_SUPER;
        KEY_MAPPING[GLFW_KEY_RIGHT_SUPER] = WindowKey.KEY_RIGHT_SUPER;
        KEY_MAPPING[GLFW_KEY_MENU] = WindowKey.KEY_MENU;


    }

    protected static final String CONTEXT_VERSION = "3.2";
    protected static final String GLSL_VERSION = "#version 420 core";
    private static boolean GLFW_INITIALIZED = false;

    private static void setupGlfw() {
        if (GLFW_INITIALIZED) return;
        if (!glfwInit()) throw new IllegalStateException("could not initialize GLFW");
        GLFW_INITIALIZED = true;
    }

    private static int cast(boolean bool) {
        return bool ? GLFW_TRUE : GLFW_FALSE;
    }

    private final WindowConfiguration config;
    private long window;

    public GlfwWindow(String title, int width, int height, WindowConfiguration config) {
        super(title, width, height);
        this.config = config;
    }

    @Override
    public void create() {
        setupGlfw();
        glfwDefaultWindowHints();
        glfwWindowHint(GLFW_VISIBLE, cast(false));


        boolean resizable = config.isResizableEnabled() && !config.isNativeFullscreenEnabled();
        glfwWindowHint(GLFW_RESIZABLE, cast(resizable));

        long monitor = 0;

        GLFWVidMode vidMode = Objects.requireNonNull(glfwGetVideoMode(glfwGetPrimaryMonitor()));

        if (config.isNativeFullscreenEnabled()) {
            monitor = glfwGetPrimaryMonitor();
            setWidth(vidMode.width());
            setHeight(vidMode.height());
        }else if(config.isBorderlessFullscreenEnabled()){
            glfwWindowHint(GLFW_RED_BITS, vidMode.redBits());
            glfwWindowHint(GLFW_GREEN_BITS, vidMode.greenBits());
            glfwWindowHint(GLFW_BLUE_BITS, vidMode.blueBits());
            glfwWindowHint(GLFW_REFRESH_RATE, vidMode.refreshRate());
            setWidth(vidMode.width());
            setHeight(vidMode.height());
            glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
        }

        String[] versionSplit = CONTEXT_VERSION.split("\\.");
        Integer major = 0;
        Integer minor = 0;
        try {
            major = Integer.parseInt(versionSplit[0]);
            if (versionSplit.length > 1) minor = Integer.parseInt(versionSplit[1]);
            else minor = 0;
        } catch (NumberFormatException e) {
            e.printStackTrace();
        }
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor);

        configureWindowHints();

        window = glfwCreateWindow(getWidth(), getHeight(), getTitle(), monitor, 0);

        if (window == 0) throw new IllegalStateException("can't create glfw window");


        if(config.isMaximizeEnabled() && !config.isNativeFullscreenEnabled() && !config.isBorderlessFullscreenEnabled()) {
            final int o1 = 31;
            final int o2 = 73;
            glfwSetWindowSize(window, vidMode.width(), vidMode.height()-o2);
            glfwSetWindowPos(window, 0,o1);
            setWidth(vidMode.width());
            setHeight(vidMode.height()-o2);
        }else{
            //center the window recompute the size
            try (MemoryStack stack = stackPush()) {
                final IntBuffer pwidth = stack.mallocInt(1);
                final IntBuffer pheight = stack.mallocInt(1);

                glfwGetWindowSize(window, pwidth, pheight);

                setWidth(pwidth.get(0));
                setHeight(pheight.get(0));

                glfwSetWindowPos(window, (vidMode.width() - pwidth.get(0)) / 2, (vidMode.height() - pheight.get(0)) / 2);
            }
        }



        glfwMakeContextCurrent(window);

        glfwSwapInterval(cast(config.isVSyncEnabled()));


        GL.createCapabilities();


        //window events
        glfwSetWindowSizeCallback(window, this::windowSizeCallback);
        glfwSetWindowFocusCallback(window, this::windowFocusCallback);
        glfwSetWindowPosCallback(window, this::windowPosCallback);

        //key listener
        glfwSetKeyCallback(window, this::keyCallback);

        //cursor position listener
        glfwSetCursorPosCallback(window, this::mousePosCallback);
        //mouse button listener
        glfwSetMouseButtonCallback(window, this::mouseButtonCallback);
        //scroll listener
        glfwSetScrollCallback(window, this::scrollCallback);

        glfwShowWindow(window);
    }

    protected void configureWindowHints(){
    }

    protected void windowSizeCallback(long window, int width, int height) {
        setWidth(width);
        setHeight(height);
        windowEventHandler.onResize(width, height);
    }

    protected void windowFocusCallback(long window, boolean focus){
        windowEventHandler.onFocusChange(focus);
    }

    protected void windowPosCallback(long window, int xpos, int ypos){
        windowEventHandler.onPositionChange(xpos, ypos);
    }

    protected void keyCallback(long window, int key, int scancode, int action, int mods){
        if(key < 0 || key >= KEY_MAPPING.length)return;
        WindowKey wKey = KEY_MAPPING[key];
        if(wKey == null) return;
        if (action == GLFW_PRESS) keyboard.keyDown(wKey);
        else if (action == GLFW_RELEASE) keyboard.keyUp(wKey);
    }

    protected void mousePosCallback(long window, double xpos, double ypos){
        mouse.onMove(xpos, ypos);
    }

    protected void mouseButtonCallback(long window, int button, int action, int mods){
        WindowButton wbutton = BUTTON_MAPPING[button];
        if(wbutton == null)return;
        if(action == GLFW_PRESS)mouse.onButtonDown(wbutton);
        else if(action == GLFW_RELEASE)mouse.onButtonUp(wbutton);
    }

    protected void scrollCallback(long window, double xOffset, double yOffset){
        mouse.onScroll(xOffset, yOffset);
    }

    @Override
    public void poolEventsImpl() {
        glfwPollEvents();
    }

    @Override
    public void swapBuffers() {
        glfwSwapBuffers(window);
    }

    @Override
    public boolean shouldClose() {
        return glfwWindowShouldClose(window);
    }

    @Override
    public void close() {
        glfwSetWindowShouldClose(window, true);
    }

    @Override
    public void dispose() {
        glfwDestroyWindow(window);
    }

    @Override
    public long pointer() {
        return window;
    }

}
