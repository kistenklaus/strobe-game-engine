package org.strobe.engine.development;

import imgui.ImGui;
import imgui.ImVec2;
import imgui.flag.ImGuiCol;
import imgui.flag.ImGuiCond;
import imgui.flag.ImGuiStyleVar;
import imgui.flag.ImGuiWindowFlags;
import imgui.internal.ImGuiDockNode;
import org.strobe.ecs.context.EntityContext;
import org.strobe.engine.StrobeEngine;
import org.strobe.gfx.Graphics;
import org.strobe.gfx.opengl.bindables.framebuffer.Framebuffer;
import org.strobe.gfx.opengl.bindables.texture.Texture2D;
import org.strobe.window.WindowContentRegion;
import org.strobe.window.WindowConfiguration;
import org.strobe.window.imgui.ImGuiWindow;

public final class DevelopmentEngine extends StrobeEngine<EntityContext> {

    private final WindowContentRegion contentRegion;

    public DevelopmentEngine(EntityContext context) {
        super(context, new ImGuiWindow(context.getTitle(), "development.ini", context.getWidth(), context.getHeight(),
                WindowConfiguration.get(false, false, true, true, false)));
        contentRegion = new WindowContentRegion(0, 0, 1, 1);
    }

    @Override
    public void afterInit(Graphics gfx) {
        gfx.getWindow().setContentRegion(contentRegion);
    }

    @Override
    protected void beforeRender(Graphics gfx) {
        drawViewportWindow(gfx);
        ImGui.showDemoWindow();

    }

    private void drawViewportWindow(Graphics gfx) {
        ImGui.begin("Viewport", ImGuiWindowFlags.NoScrollbar | ImGuiWindowFlags.NoScrollWithMouse);

        if(ImGui.beginTabBar("tab bar")){
            if(ImGui.beginTabItem("default")){
                drawViewportImage(gfx);
                ImGui.endTabItem();
            }
            if(ImGui.beginTabItem("none")){
                ImGui.endTabItem();
            }


            ImGui.endTabBar();
        }

        ImGui.end();
    }

    private void drawViewportImage(Graphics gfx){
        ImVec2 panelSize = ImGui.getContentRegionAvail();
        panelSize.x -= ImGui.getScrollX();
        panelSize.y -= ImGui.getScrollY();
        float yOffset = ImGui.getCursorPosY();

        //calculate the size of the viewport
        float viewportAspect = context.getWidth() / (float) context.getHeight();
        float viewportWidth = panelSize.x;
        float viewportHeight = viewportWidth / viewportAspect;
        if (viewportHeight > panelSize.y) {
            viewportHeight = panelSize.y;
            viewportWidth = viewportHeight * viewportAspect;
        }
        //calculate the center position of the viewport
        float viewportX = (panelSize.x / 2.0f) - (viewportWidth / 2.0f);
        float viewportY = (panelSize.y / 2.0f) - (viewportHeight / 2.0f)+yOffset;


        float viewportWindowPosX = ((ImGui.getWindowPosX() + viewportX) / gfx.getWidth()) * 2 - 1;
        float viewportWindowPosY = ((ImGui.getWindowPosY() + viewportY) / gfx.getHeight()) * 2 - 1;
        contentRegion.updateContentRegion(
                viewportWindowPosX,
                viewportWindowPosY,
                (viewportWidth / gfx.getWidth()) * 2,
                (viewportHeight / gfx.getHeight()) * 2);


        ImGui.setCursorPos(viewportX, viewportY);
        Texture2D viewportTex = context.getTarget().getAttachmentTexture(Framebuffer.Attachment.COLOR_RGB_ATTACHMENT_0);

        ImGui.pushStyleColor(ImGuiCol.Border, ImGui.getColorU32(1,1,1,1));
        ImGui.pushStyleVar(ImGuiStyleVar.PopupBorderSize, 10);

        ImGui.image(viewportTex.getID(), viewportWidth, viewportHeight, 0, 1, 1, 0,1,1,1,1,1,1,1,1);

        ImGui.popStyleVar();
        ImGui.popStyleColor();

    }

    private void drawTestingWindow(String name) {
        ImGui.begin(name);
        ImGui.end();
    }
}
