package org.strobe.window.imgui;

import imgui.ImGui;
import imgui.ImGuiIO;
import imgui.flag.ImGuiCond;
import imgui.flag.ImGuiConfigFlags;
import imgui.flag.ImGuiStyleVar;
import imgui.flag.ImGuiWindowFlags;
import imgui.gl3.ImGuiImplGl3;
import imgui.glfw.ImGuiImplGlfw;
import imgui.type.ImBoolean;
import org.strobe.window.glfw.GlfwWindow;

import static org.lwjgl.glfw.GLFW.*;

public final class ImGuiWindow extends GlfwWindow {

    private final ImGuiImplGl3 imGuiGl3 = new ImGuiImplGl3();
    private final ImGuiImplGlfw imGuiGlfw = new ImGuiImplGlfw();

    public ImGuiWindow(String title, int width, int height) {
        super(title, width, height);
    }

    @Override
    protected void configureWindowHints() {
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
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
        io.setConfigViewportsNoTaskBarIcon(true);
        //wtf does that do joinked that

        //init imGuiGlfw
        imGuiGlfw.init(pointer(), true);
        imGuiGl3.init(GLSL_VERSION);


        newFrame();
    }

    private void newFrame(){
        imGuiGlfw.newFrame();
        ImGui.newFrame();
        beginDockSpace();;
    }

    private void endFrame(){
        endDockSpace();
        ImGui.render();
        imGuiGl3.renderDrawData(ImGui.getDrawData());

        if (ImGui.getIO().hasConfigFlags(ImGuiConfigFlags.ViewportsEnable)) {
            final long backupWindowPtr = glfwGetCurrentContext();
            ImGui.updatePlatformWindows();
            ImGui.renderPlatformWindowsDefault();
            glfwMakeContextCurrent(backupWindowPtr);
        }
    }

    @Override
    public void swapBuffers() {
        endFrame();
        super.swapBuffers();
        newFrame();
    }

    private void beginDockSpace(){
        int windowFlags =
                ImGuiWindowFlags.MenuBar | ImGuiWindowFlags.NoDocking |
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
