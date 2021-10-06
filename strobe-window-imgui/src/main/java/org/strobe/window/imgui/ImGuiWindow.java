package org.strobe.window.imgui;

import imgui.ImGui;
import imgui.ImGuiIO;
import imgui.flag.*;
import imgui.gl3.ImGuiImplGl3;
import imgui.glfw.ImGuiImplGlfw;
import imgui.type.ImBoolean;
import org.strobe.window.WindowConfiguration;
import org.strobe.window.WindowKey;
import org.strobe.window.WindowKeyListener;
import org.strobe.window.glfw.GlfwWindow;

import static org.lwjgl.glfw.GLFW.*;

public final class ImGuiWindow extends GlfwWindow {

    private final static int[] IMGUI_KEY_MAP = new int[ImGuiKey.COUNT];
    static{
        IMGUI_KEY_MAP[ImGuiKey.Tab] = WindowKey.KEY_TAB.ordinal();
        IMGUI_KEY_MAP[ImGuiKey.LeftArrow] = WindowKey.KEY_LEFT.ordinal();
        IMGUI_KEY_MAP[ImGuiKey.RightArrow] = WindowKey.KEY_RIGHT.ordinal();
        IMGUI_KEY_MAP[ImGuiKey.UpArrow] = WindowKey.KEY_UP.ordinal();
        IMGUI_KEY_MAP[ImGuiKey.DownArrow] = WindowKey.KEY_DOWN.ordinal();
        IMGUI_KEY_MAP[ImGuiKey.PageUp] = WindowKey.KEY_PAGE_UP.ordinal();
        IMGUI_KEY_MAP[ImGuiKey.PageDown] = WindowKey.KEY_PAGE_DOWN.ordinal();
        IMGUI_KEY_MAP[ImGuiKey.Home] = WindowKey.KEY_HOME.ordinal();
        IMGUI_KEY_MAP[ImGuiKey.End] = WindowKey.KEY_END.ordinal();
        IMGUI_KEY_MAP[ImGuiKey.Insert] = WindowKey.KEY_INSERT.ordinal();
        IMGUI_KEY_MAP[ImGuiKey.Delete] = WindowKey.KEY_DELETE.ordinal();
        IMGUI_KEY_MAP[ImGuiKey.Backspace] = WindowKey.KEY_BACKSPACE.ordinal();
        IMGUI_KEY_MAP[ImGuiKey.Space] = WindowKey.KEY_SPACE.ordinal();
        IMGUI_KEY_MAP[ImGuiKey.Enter] = WindowKey.KEY_ENTER.ordinal();
        IMGUI_KEY_MAP[ImGuiKey.Escape] = WindowKey.KEY_ESCAPE.ordinal();
        //IMGUI_KEY_MAP[ImGuiKey.KeyPadEnter] = WindowKey.KEY_ENTER.ordinal();
        IMGUI_KEY_MAP[ImGuiKey.A] = WindowKey.KEY_A.ordinal();
        IMGUI_KEY_MAP[ImGuiKey.C] = WindowKey.KEY_C.ordinal();
        IMGUI_KEY_MAP[ImGuiKey.V] = WindowKey.KEY_V.ordinal();
        IMGUI_KEY_MAP[ImGuiKey.X] = WindowKey.KEY_X.ordinal();
        IMGUI_KEY_MAP[ImGuiKey.Y] = WindowKey.KEY_Y.ordinal();
        IMGUI_KEY_MAP[ImGuiKey.Z] = WindowKey.KEY_Z.ordinal();
    }

    private final ImGuiImplGl3 imGuiGl3 = new ImGuiImplGl3();
    private final ImGuiImplGlfw imGuiGlfw = new ImGuiImplGlfw();

    public ImGuiWindow(String title, int width, int height, WindowConfiguration config) {
        super(title, width, height, config);
    }

    @Override
    public void create() {
        super.create();

        //init ImGui
        ImGui.createContext();

        final ImGuiIO io = ImGui.getIO();
        io.addConfigFlags(ImGuiConfigFlags.NavEnableKeyboard);
        io.addConfigFlags(ImGuiConfigFlags.DockingEnable);
        //io.addConfigFlags(ImGuiConfigFlags.ViewportsEnable);

        io.setBackendFlags(ImGuiBackendFlags.HasMouseCursors);
        io.setBackendPlatformName("imgui_java_impl_glfw");
        //TODO keyboard map
        io.setKeyMap(IMGUI_KEY_MAP);

        //TODO mouse button map

        //init imGuiGlfw
        imGuiGlfw.init(pointer(), true);
        imGuiGl3.init(GLSL_VERSION);

    }


    public void newFrame(){
        pollEvents();
        imGuiGlfw.newFrame();
        ImGui.newFrame();
        beginDockSpace();;
    }

    public void endFrame(){
        endDockSpace();
        ImGui.render();
        imGuiGl3.renderDrawData(ImGui.getDrawData());

        if (ImGui.getIO().hasConfigFlags(ImGuiConfigFlags.ViewportsEnable)) {
            final long backupWindowPtr = glfwGetCurrentContext();
            ImGui.updatePlatformWindows();
            ImGui.renderPlatformWindowsDefault();
            glfwMakeContextCurrent(backupWindowPtr);
        }
        swapBuffers();
    }


    private void beginDockSpace(){
        int windowFlags =
                ImGuiWindowFlags.NoDecoration | ImGuiWindowFlags.NoDocking |
                        ImGuiWindowFlags.NoTitleBar | ImGuiWindowFlags.NoCollapse |
                        ImGuiWindowFlags.NoResize | ImGuiWindowFlags.NoMove |
                        ImGuiWindowFlags.NoBringToFrontOnFocus | ImGuiWindowFlags.NoNavFocus|
                ImGuiWindowFlags.NoNav;
        ImGui.setNextWindowPos(0,0, ImGuiCond.Always);
        ImGui.setNextWindowSize(getWidth(), getHeight(), ImGuiCond.Always);
        ImGui.pushStyleVar(ImGuiStyleVar.WindowRounding, 0.0f);
        ImGui.pushStyleVar(ImGuiStyleVar.WindowBorderSize, 0.0f);

        ImGui.begin("Dock space Window", new ImBoolean(true), windowFlags);
        ImGui.popStyleVar(2);

        ImGui.dockSpace(ImGui.getID("Dockspace"));
    }

    private void endDockSpace(){
        ImGui.end();
    }

    @Override
    public void dispose() {
        super.dispose();
        imGuiGl3.dispose();
        imGuiGlfw.dispose();
        ImGui.destroyContext();
    }
}
