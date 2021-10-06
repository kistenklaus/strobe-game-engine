package org.strobe.engine.development;

import imgui.ImGui;
import imgui.ImVec2;
import imgui.flag.ImGuiWindowFlags;
import org.strobe.ecs.context.EntityContext;
import org.strobe.engine.StrobeEngine;
import org.strobe.gfx.Graphics;
import org.strobe.gfx.opengl.OpenGlContext;
import org.strobe.gfx.opengl.bindables.framebuffer.Framebuffer;
import org.strobe.gfx.opengl.bindables.texture.Texture2D;
import org.strobe.window.WindowConfiguration;
import org.strobe.window.imgui.ImGuiWindow;

public final class DevelopmentEngine extends StrobeEngine<EntityContext> {

    public DevelopmentEngine(EntityContext context) {
        super(context, new ImGuiWindow(context.getTitle(), context.getWidth(), context.getHeight(),
                WindowConfiguration.get(false, false, true, true, false)));
    }

    @Override
    protected void beforeRender(Graphics gfx) {
        drawViewportWindow();
        drawTestingWindow("abc");
        drawTestingWindow("xyz");
        drawTestingWindow("etc");
        drawTestingWindow("duh");
    }

    private void drawViewportWindow(){
        ImGui.begin("Viewport", ImGuiWindowFlags.NoScrollbar | ImGuiWindowFlags.NoScrollWithMouse);

        ImVec2 panelSize = ImGui.getContentRegionAvail();
        panelSize.x -= ImGui.getScrollX();
        panelSize.y -= ImGui.getScrollY();

        //calculate the size of the viewport
        float viewportAspect = context.getWidth() / (float)context.getHeight();
        float viewportWidth = panelSize.x;
        float viewportHeight = viewportWidth/viewportAspect;
        if(viewportHeight > panelSize.y){
            viewportHeight = panelSize.y;
            viewportWidth = viewportHeight * viewportAspect;
        }
        //calculate the center position of the viewport
        float viewportX = (panelSize.x / 2.0f) - (viewportWidth / 2.0f);
        float viewportY = (panelSize.y / 2.0f) - (viewportHeight / 2.0f);

        ImGui.setCursorPos(viewportX, viewportY);
        Texture2D viewportTex = context.getTarget().getAttachmentTexture(Framebuffer.Attachment.COLOR_RGB_ATTACHMENT_0);
        ImGui.image(viewportTex.getID(), viewportWidth,viewportHeight, 0,1,1,0);

        ImGui.end();
    }

    private void drawTestingWindow(String name){
        ImGui.begin(name);
        ImGui.end();
    }
}
