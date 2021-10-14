package org.strobe.window.imgui;

import imgui.*;
import imgui.callback.ImStrConsumer;
import imgui.callback.ImStrSupplier;
import imgui.flag.*;
import imgui.gl3.ImGuiImplGl3;
import imgui.type.ImBoolean;
import org.lwjgl.PointerBuffer;
import org.lwjgl.glfw.GLFWErrorCallbackI;
import org.lwjgl.glfw.GLFWNativeWin32;
import org.lwjgl.glfw.GLFWVidMode;
import org.strobe.utils.ResourceLoader;
import org.strobe.window.WindowConfiguration;
import org.strobe.window.glfw.GlfwWindow;

import java.io.IOException;
import java.nio.ByteBuffer;

import static org.lwjgl.glfw.GLFW.*;

public class ImGuiWindow extends GlfwWindow {

    private static final boolean IS_WINDOWS;

    static {
        String OS = System.getProperty("os.name", "generic").toLowerCase();
        IS_WINDOWS = OS.contains("win");
    }

    private final static int[] IMGUI_KEY_MAP = new int[ImGuiKey.COUNT];

    static {
        IMGUI_KEY_MAP[ImGuiKey.Tab] = GLFW_KEY_TAB;
        IMGUI_KEY_MAP[ImGuiKey.LeftArrow] = GLFW_KEY_LEFT;
        IMGUI_KEY_MAP[ImGuiKey.RightArrow] = GLFW_KEY_RIGHT;
        IMGUI_KEY_MAP[ImGuiKey.UpArrow] = GLFW_KEY_UP;
        IMGUI_KEY_MAP[ImGuiKey.DownArrow] = GLFW_KEY_DOWN;
        IMGUI_KEY_MAP[ImGuiKey.PageUp] = GLFW_KEY_PAGE_UP;
        IMGUI_KEY_MAP[ImGuiKey.PageDown] = GLFW_KEY_PAGE_DOWN;
        IMGUI_KEY_MAP[ImGuiKey.Home] = GLFW_KEY_HOME;
        IMGUI_KEY_MAP[ImGuiKey.End] = GLFW_KEY_END;
        IMGUI_KEY_MAP[ImGuiKey.Insert] = GLFW_KEY_INSERT;
        IMGUI_KEY_MAP[ImGuiKey.Delete] = GLFW_KEY_DELETE;
        IMGUI_KEY_MAP[ImGuiKey.Backspace] = GLFW_KEY_BACKSPACE;
        IMGUI_KEY_MAP[ImGuiKey.Space] = GLFW_KEY_SPACE;
        IMGUI_KEY_MAP[ImGuiKey.Enter] = GLFW_KEY_ENTER;
        IMGUI_KEY_MAP[ImGuiKey.Escape] = GLFW_KEY_ESCAPE;
        IMGUI_KEY_MAP[ImGuiKey.KeyPadEnter] = GLFW_KEY_KP_ENTER;
        IMGUI_KEY_MAP[ImGuiKey.A] = GLFW_KEY_A;
        IMGUI_KEY_MAP[ImGuiKey.C] = GLFW_KEY_C;
        IMGUI_KEY_MAP[ImGuiKey.V] = GLFW_KEY_V;
        IMGUI_KEY_MAP[ImGuiKey.X] = GLFW_KEY_X;
        IMGUI_KEY_MAP[ImGuiKey.Y] = GLFW_KEY_Y;
        IMGUI_KEY_MAP[ImGuiKey.Z] = GLFW_KEY_Z;
    }

    private final long[] mouseCursors = new long[ImGuiMouseCursor.COUNT];

    private boolean glfwHasPerMonitorDpi;
    private boolean glfwHasMonitorWorkArea;

    protected final ImGuiImplGl3 imGuiGl3 = new ImGuiImplGl3();
    private final int[] monitorX = new int[1];
    private final int[] monitorY = new int[1];
    private final int[] monitorWorkAreaX = new int[1];
    private final int[] monitorWorkAreaY = new int[1];
    private final int[] monitorWorkAreaWidth = new int[1];
    private final int[] monitorWorkAreaHeight = new int[1];
    private final float[] monitorContentScaleX = new float[1];
    private final float[] monitorContentScaleY = new float[1];
    private boolean wantUpdateMonitors = true;

    private final int[] windowX = new int[1];
    private final int[] windowY = new int[1];
    private final double[] mousePos = new double[2];

    private double time = 0.0f;

    private final String iniFile;
    private final String fontResourcePath;

    public ImGuiWindow(String title, String iniFile, int width, int height, WindowConfiguration config, String fontResourcePath) {
        super(title, width, height, config);
        this.iniFile = iniFile;
        this.fontResourcePath = fontResourcePath;
    }

    @Override
    public void create() {
        super.create();

        ImGui.createContext();

        detectGlfwVersionAndEnabledFeatures();

        final ImGuiIO io = ImGui.getIO();
        io.setIniFilename(iniFile);
        io.addConfigFlags(ImGuiConfigFlags.NavEnableKeyboard | ImGuiConfigFlags.DockingEnable);
        io.addBackendFlags(ImGuiBackendFlags.HasMouseCursors | ImGuiBackendFlags.HasSetMousePos | ImGuiBackendFlags.PlatformHasViewports);
        io.setBackendPlatformName("imgui_window_glfw_impl");

        io.setKeyMap(IMGUI_KEY_MAP);

        //disable error callback while setting up glfw cursors
        final GLFWErrorCallbackI errorCallback = glfwSetErrorCallback(null);
        mouseCursors[ImGuiMouseCursor.Arrow] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
        mouseCursors[ImGuiMouseCursor.TextInput] = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
        mouseCursors[ImGuiMouseCursor.ResizeAll] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
        mouseCursors[ImGuiMouseCursor.ResizeNS] = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
        mouseCursors[ImGuiMouseCursor.ResizeEW] = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
        mouseCursors[ImGuiMouseCursor.ResizeNESW] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
        mouseCursors[ImGuiMouseCursor.ResizeNWSE] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
        mouseCursors[ImGuiMouseCursor.Hand] = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
        mouseCursors[ImGuiMouseCursor.NotAllowed] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
        glfwSetErrorCallback(errorCallback);

        io.setGetClipboardTextFn(new ImStrSupplier() {
            @Override
            public String get() {
                final String clipboardString = glfwGetClipboardString(pointer());
                return clipboardString != null ? clipboardString : "";
            }
        });

        io.setSetClipboardTextFn(new ImStrConsumer() {
            @Override
            public void accept(final String str) {
                glfwSetClipboardString(pointer(), str);
            }
        });

        //install mouse callbacks
        glfwSetCharCallback(pointer(), this::charCallback);
        glfwSetMonitorCallback(this::monitorCallback);
        updateMonitors();
        //I think we do this because of a glitch in glfw
        glfwSetMonitorCallback(this::monitorCallback);

        final ImGuiViewport imGuiViewport = ImGui.getMainViewport();
        imGuiViewport.setPlatformHandle(pointer());

        if (IS_WINDOWS) {
            imGuiViewport.setPlatformHandleRaw(GLFWNativeWin32.glfwGetWin32Window(pointer()));
        }

        if(fontResourcePath != null){
            final ImFontGlyphRangesBuilder rangesBuilder = new ImFontGlyphRangesBuilder();
            rangesBuilder.addRanges(io.getFonts().getGlyphRangesDefault());
            final short[] glyphRanges = rangesBuilder.buildRanges();

            final ImFontConfig fontConfig = new ImFontConfig();
            fontConfig.setGlyphRanges(glyphRanges);
            fontConfig.setMergeMode(false);
            fontConfig.setPixelSnapH(false);


            try {
                ByteBuffer ttfBuffer = ResourceLoader.ioResourceToByteBuffer(fontResourcePath);
                byte[] ttf = new byte[ttfBuffer.capacity()];
                ttfBuffer.position(0);
                for (int i = 0; i < ttfBuffer.capacity(); i++) ttf[i] = ttfBuffer.get();

                io.getFonts().addFontFromMemoryTTF(ttf, 16.0f, fontConfig,glyphRanges);
                io.getFonts().addFontDefault();

                io.getFonts().build();
            } catch (IOException e) {
                e.printStackTrace();
            }
            fontConfig.destroy();
        }


        imGuiGl3.init(GLSL_VERSION);

        io.setDisplaySize(getWidth(), getHeight());
    }

    private void detectGlfwVersionAndEnabledFeatures() {
        final int[] major = new int[1];
        final int[] minor = new int[1];
        final int[] rev = new int[1];
        glfwGetVersion(major, minor, rev);

        final int version = major[0] * 1000 + minor[0] * 100 + rev[0] * 10;

        glfwHasPerMonitorDpi = version >= 3300;
        glfwHasMonitorWorkArea = version >= 3300;
    }

    @Override
    protected void mousePosCallback(long window, double xpos, double ypos) {
        super.mousePosCallback(window, xpos, ypos);
        mousePos[0] = xpos;
        mousePos[1] = ypos;
    }

    protected void scrollCallback(long window, double xOffset, double yOffset) {
        super.scrollCallback(window, xOffset, yOffset);
        final ImGuiIO io = ImGui.getIO();
        io.setMouseWheelH(io.getMouseWheelH() + (float) xOffset);
        io.setMouseWheel(io.getMouseWheel() + (float) yOffset);
    }

    protected void keyCallback(long window, int key, int scancode, int action, int mods) {
        super.keyCallback(window, key, scancode, action, mods);

        final ImGuiIO io = ImGui.getIO();
        if (key >= 0) {
            if (action == GLFW_PRESS) {
                io.setKeysDown(key, true);
            } else if (action == GLFW_RELEASE) {
                io.setKeysDown(key, false);
            }
        }

        io.setKeyCtrl(io.getKeysDown(GLFW_KEY_LEFT_CONTROL) || io.getKeysDown(GLFW_KEY_RIGHT_CONTROL));
        io.setKeyShift(io.getKeysDown(GLFW_KEY_LEFT_SHIFT) || io.getKeysDown(GLFW_KEY_RIGHT_SHIFT));
        io.setKeyAlt(io.getKeysDown(GLFW_KEY_LEFT_ALT) || io.getKeysDown(GLFW_KEY_RIGHT_ALT));
        io.setKeySuper(io.getKeysDown(GLFW_KEY_LEFT_SUPER) || io.getKeysDown(GLFW_KEY_RIGHT_SUPER));
    }

    private void charCallback(long window, int c) {
        final ImGuiIO io = ImGui.getIO();
        io.addInputCharacter(c);
    }

    private void monitorCallback(long window, int event) {
        wantUpdateMonitors = true;
    }

    private void updateMonitors() {
        final ImGuiPlatformIO platformIO = ImGui.getPlatformIO();
        final PointerBuffer monitors = glfwGetMonitors();

        platformIO.resizeMonitors(0);

        for (int n = 0; n < monitors.limit(); n++) {
            final long monitor = monitors.get(n);

            glfwGetMonitorPos(monitor, monitorX, monitorY);
            final GLFWVidMode vidMode = glfwGetVideoMode(monitor);
            final float mainPosX = monitorX[0];
            final float mainPosY = monitorY[0];
            final float mainSizeX = vidMode.width();
            final float mainSizeY = vidMode.height();

            if (glfwHasMonitorWorkArea) {
                glfwGetMonitorWorkarea(monitor, monitorWorkAreaX, monitorWorkAreaY, monitorWorkAreaWidth, monitorWorkAreaHeight);
            }

            float workPosX = 0;
            float workPosY = 0;
            float workSizeX = 0;
            float workSizeY = 0;

            // Workaround a small GLFW issue reporting zero on monitor changes: https://github.com/glfw/glfw/pull/1761
            if (glfwHasMonitorWorkArea && monitorWorkAreaWidth[0] > 0 && monitorWorkAreaHeight[0] > 0) {
                workPosX = monitorWorkAreaX[0];
                workPosY = monitorWorkAreaY[0];
                workSizeX = monitorWorkAreaWidth[0];
                workSizeY = monitorWorkAreaHeight[0];
            }

            // Warning: the validity of monitor DPI information on Windows depends on the application DPI awareness settings,
            // which generally needs to be set in the manifest or at runtime.
            if (glfwHasPerMonitorDpi) {
                glfwGetMonitorContentScale(monitor, monitorContentScaleX, monitorContentScaleY);
            }
            final float dpiScale = monitorContentScaleX[0];

            platformIO.pushMonitors(mainPosX, mainPosY, mainSizeX, mainSizeY, workPosX, workPosY, workSizeX, workSizeY, dpiScale);
        }

        wantUpdateMonitors = false;
    }

    public void newFrame() {
        pollEvents();

        final ImGuiIO io = ImGui.getIO();
        if (!io.getFonts().isBuilt()) {
            throw new IllegalStateException("could not build font");
        }

        io.setDisplaySize(getWidth(), getHeight());
        io.setDisplayFramebufferScale(1f, 1f);

        final double currentTime = glfwGetTime();
        io.setDeltaTime(time > 0.0 ? (float) (currentTime - time) : 1.0f / 60.0f);
        time = currentTime;

        if (wantUpdateMonitors) updateMonitors();

        updateMouse();

        ImGui.newFrame();
        beginDockSpace();
    }

    private void updateMouse() {
        final ImGuiIO io = ImGui.getIO();


        //update position and buttons
        final ImVec2 mousePosBackup = new ImVec2();
        io.getMousePos(mousePosBackup);
        io.setMousePos(-Float.MAX_VALUE, -Float.MAX_VALUE);
        io.setMouseHoveredViewport(0);

        final ImGuiPlatformIO platformIO = ImGui.getPlatformIO();

        for (int n = 0; n < platformIO.getViewportsSize(); n++) {
            final ImGuiViewport viewport = platformIO.getViewports(n);
            final long windowPtr = viewport.getPlatformHandle();

            final boolean focused = glfwGetWindowAttrib(windowPtr, GLFW_FOCUSED) != 0;

            if (focused) {
                if (io.getWantSetMousePos()) {
                    glfwSetCursorPos(windowPtr, mousePosBackup.x - viewport.getPosX(), mousePosBackup.y - viewport.getPosY());
                } else {
                    if (io.hasConfigFlags(ImGuiConfigFlags.ViewportsEnable)) {
                        // Multi-viewport mode: mouse position in OS absolute coordinates (io.MousePos is (0,0) when the mouse is on the upper-left of the primary monitor)
                        glfwGetWindowPos(windowPtr, windowX, windowY);
                        io.setMousePos((float) mousePos[0] + windowX[0], (float) mousePos[1] + windowY[0]);
                    } else {
                        // Single viewport mode: mouse position in client window coordinates (io.MousePos is (0,0) when the mouse is on the upper-left corner of the app window)
                        io.setMousePos((float) mousePos[0], (float) mousePos[1]);
                    }
                }

                for (int i = 0; i < ImGuiMouseButton.COUNT; i++) {
                    if (contentRegion != null && !contentRegion.isContentFocused()) {
                        io.setMouseDown(i, glfwGetMouseButton(windowPtr, i) != 0);
                    }
                }
            }
        }


        //update cursors
        final boolean noCursorChange = io.hasConfigFlags(ImGuiConfigFlags.NoMouseCursorChange);
        final boolean cursorDisabled = glfwGetInputMode(pointer(), GLFW_CURSOR) == GLFW_CURSOR_DISABLED;
        if (noCursorChange || cursorDisabled) return;

        final int imguiCursor = ImGui.getMouseCursor();

        for (int n = 0; n < platformIO.getViewportsSize(); n++) {
            final long windowPtr = platformIO.getViewports(n).getPlatformHandle();

            if (imguiCursor == ImGuiMouseCursor.None || io.getMouseDrawCursor()) {
                // Hide OS mouse cursor if imgui is drawing it or if it wants no cursor
                glfwSetInputMode(windowPtr, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
            } else {
                // Show OS mouse cursor
                // FIXME-PLATFORM: Unfocused windows seems to fail changing the mouse cursor with GLFW 3.2, but 3.3 works here.
                glfwSetCursor(windowPtr, mouseCursors[imguiCursor] != 0 ? mouseCursors[imguiCursor] : mouseCursors[ImGuiMouseCursor.Arrow]);
                glfwSetInputMode(windowPtr, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
        }


    }

    public void endFrame() {
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

    protected void beginDockSpace() {
        int windowFlags =
                ImGuiWindowFlags.NoDecoration | ImGuiWindowFlags.NoDocking |
                        ImGuiWindowFlags.NoTitleBar | ImGuiWindowFlags.NoCollapse |
                        ImGuiWindowFlags.NoResize | ImGuiWindowFlags.NoMove |
                        ImGuiWindowFlags.NoBringToFrontOnFocus | ImGuiWindowFlags.NoNavFocus |
                        ImGuiWindowFlags.NoNav;
        ImGui.setNextWindowPos(0, 0, ImGuiCond.Always);
        ImGui.setNextWindowSize(getWidth(), getHeight(), ImGuiCond.Always);
        ImGui.pushStyleVar(ImGuiStyleVar.WindowRounding, 0.0f);
        ImGui.pushStyleVar(ImGuiStyleVar.WindowBorderSize, 0.0f);

        ImGui.begin("Dock space Window", new ImBoolean(true), windowFlags);
        ImGui.popStyleVar(2);

        ImGui.dockSpace(ImGui.getID("Dockspace"));
    }

    protected void endDockSpace() {
        ImGui.end();
    }

    @Override
    public void dispose() {
        super.dispose();
        imGuiGl3.dispose();
        //imGuiGlfw.dispose();
        ImGui.destroyContext();
    }

}
