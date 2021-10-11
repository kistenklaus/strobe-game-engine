package org.strobe.engine.development;

import imgui.ImGui;
import imgui.flag.ImGuiCol;
import imgui.flag.ImGuiStyleVar;
import org.strobe.ecs.Entity;
import org.strobe.gfx.Graphics;
import org.strobe.gfx.opengl.bindables.texture.Texture2D;
import org.strobe.gfx.opengl.bindables.texture.TextureLoader;
import org.strobe.window.imgui.ImGuiStyle;

import java.awt.*;

public class DevelopmentStyle implements ImGuiStyle {


    private final String fontResource = "fonts/JetBrainsMono/JetBrainsMono-SemiBold.ttf";

    private final Color windowColorAWT = new Color(36, 75, 199);
    private int windowColor;
    private final Color mainColorAWT = new Color(128, 128, 128);
    private int mainColor;
    private final Color darkColorAWT = new Color(82, 82, 82);
    private int darkColor;
    private final Color brightColorAWT = new Color(175, 175, 175);
    private int brightColor;


    private Texture2D closedTreeIcon;
    private Texture2D openTreeIcon;
    private Texture2D renderableEntityIcon;
    private Texture2D emptyEntityIcon;
    private Texture2D transformEntityIcon;

    public void init(Graphics gfx){
        darkColor = ImGui.getColorU32(darkColorAWT.getRed() / 255f, darkColorAWT.getGreen() / 255f, darkColorAWT.getGreen() / 255f, darkColorAWT.getAlpha() / 255f);
        brightColor = ImGui.getColorU32(brightColorAWT.getRed() / 255f, brightColorAWT.getGreen() / 255f, brightColorAWT.getBlue() / 255f, brightColorAWT.getAlpha() / 255f);
        mainColor = ImGui.getColorU32(mainColorAWT.getRed() / 255f, mainColorAWT.getGreen() / 255f, mainColorAWT.getBlue() / 255f, mainColorAWT.getAlpha() / 255f);
        windowColor = ImGui.getColorU32(windowColorAWT.getRed() / 255f, windowColorAWT.getGreen() / 255f, windowColorAWT.getBlue() / 255f, windowColorAWT.getAlpha() / 255f);



        renderableEntityIcon = TextureLoader.loadTexture(gfx, "icons/solidtile.png");
        closedTreeIcon = TextureLoader.loadTexture(gfx, "icons/closed.png");
        openTreeIcon = TextureLoader.loadTexture(gfx, "icons/open.png");
        emptyEntityIcon = null;
        transformEntityIcon = null;
    }

    @Override
    public String getFontResource() {
        return fontResource;
    }

    @Override
    public void push() {

        ImGui.pushStyleColor(ImGuiCol.ChildBg, mainColor);

        ImGui.pushStyleColor(ImGuiCol.Tab, brightColor);
        ImGui.pushStyleColor(ImGuiCol.TabActive, darkColor);
        ImGui.pushStyleColor(ImGuiCol.TabHovered, darkColor);
        ImGui.pushStyleColor(ImGuiCol.TabUnfocused, darkColor);
        ImGui.pushStyleColor(ImGuiCol.TabUnfocusedActive, brightColor);

        ImGui.pushStyleColor(ImGuiCol.MenuBarBg, darkColor);

        ImGui.pushStyleColor(ImGuiCol.FrameBg, darkColor);

        ImGui.pushStyleColor(ImGuiCol.Separator, darkColor);

        ImGui.pushStyleVar(ImGuiStyleVar.FrameRounding, 2);
    }

    @Override
    public void pop() {
        ImGui.popStyleVar();
        ImGui.popStyleColor(9);
    }

    public int getDarkColor() {
        return darkColor;
    }

    public int getBrightColor() {
        return brightColor;
    }

    public Color getWindowColorAWT() {
        return windowColorAWT;
    }

    public int getWindowColor() {
        return windowColor;
    }

    public int getMainColor() {
        return mainColor;
    }

    public Color getMainColorAWT() {
        return mainColorAWT;
    }

    public Color getDarkColorAWT() {
        return darkColorAWT;
    }

    public Color getBrightColorAWT() {
        return brightColorAWT;
    }

    public Texture2D getClosedTreeIcon() {
        return closedTreeIcon;
    }

    public Texture2D getOpenTreeIcon() {
        return openTreeIcon;
    }

    public Texture2D getRenderableEntityIcon() {
        return renderableEntityIcon;
    }

    public Texture2D getEmptyEntityIcon() {
        return emptyEntityIcon;
    }

    public Texture2D getTransformEntityIcon() {
        return transformEntityIcon;
    }

    public Texture2D getEntityIcon(Entity entity){
        return renderableEntityIcon;
    }
}
