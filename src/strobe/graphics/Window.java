package strobe.graphics;

import static org.lwjgl.glfw.GLFW.*;
import static org.lwjgl.opengl.GL11.*;
import static org.lwjgl.system.MemoryStack.stackPush;
import static org.lwjgl.opengl.GL46.GL_CONTEXT_FLAG_NO_ERROR_BIT;
import static org.lwjgl.system.MemoryUtil.NULL;

import org.lwjgl.glfw.GLFWErrorCallback;
import org.lwjgl.glfw.GLFWVidMode;
import org.lwjgl.opengl.GL;
import org.lwjgl.system.MemoryStack;
import strobe.StrobeException;

import java.nio.IntBuffer;
import java.util.Objects;

public class Window {

    private final long window;
    private String title;
    private boolean fullscreen;
    private int window_width,window_height;

    public Window(String title, int window_width, int window_height, boolean fullscreen){
        this.title = title;
        GLFWErrorCallback.createPrint(System.err).set();
        if(!glfwInit()){
            throw new StrobeException("Failed to initialize GLFW");
        }
        glfwDefaultWindowHints();
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        //glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        this.fullscreen = fullscreen;
        this.window_width = window_width;
        this.window_height = window_height;

        long monitor = NULL;
        if(fullscreen){
            monitor = glfwGetPrimaryMonitor();
            GLFWVidMode mode = glfwGetVideoMode(monitor);
            this.window_width = mode.width();
            this.window_height = mode.height();
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

        window = glfwCreateWindow(window_width, window_height, title, monitor, NULL);

        if(window == NULL){
            throw new StrobeException("Failed to create GLFW-Window");
        }

        try(MemoryStack stack = stackPush()){
            final IntBuffer pwidth = stack.mallocInt(1);
            final IntBuffer pheight = stack.mallocInt(1);

            glfwGetWindowSize(window,pwidth, pheight);

            final GLFWVidMode vidmode = Objects.requireNonNull(glfwGetVideoMode(glfwGetPrimaryMonitor()));
            glfwSetWindowPos(window, (vidmode.width() - pwidth.get(0))/2,( vidmode.height()-pheight.get(0))/2);
        }

        glfwMakeContextCurrent(window);
        glfwSwapInterval(GL_TRUE);
        glfwShowWindow(window);
        GL.createCapabilities();

    }

    public boolean shouldClose(){
        return glfwWindowShouldClose(window);
    }
    public void pollEvents(){
        glfwPollEvents();
    }
    public void close(){
        glfwSetWindowShouldClose(window, true);
    }
    public void swapBuffers(){
        glfwSwapBuffers(window);
    }
    public void dispose(){
        glfwDestroyWindow(window);
    }
    public int getWidth(){
        return window_width;
    }
    public int getHeight(){
        return window_height;
    }
    public long pointer(){
        return this.window;
    }

}
