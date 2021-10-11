package org.strobe.engine.development;

import imgui.ImGui;
import imgui.ImGuiIO;
import imgui.ImVec2;
import imgui.flag.ImGuiCol;
import imgui.flag.ImGuiStyleVar;
import imgui.flag.ImGuiWindowFlags;
import org.joml.Vector4f;
import org.strobe.ecs.Entity;
import org.strobe.ecs.EntityComponentSystem;
import org.strobe.ecs.context.EntityContext;
import org.strobe.gfx.Graphics;
import org.strobe.gfx.opengl.bindables.texture.Texture2D;
import org.strobe.gfx.opengl.bindables.texture.TextureLoader;
import org.strobe.window.imgui.ImGuiUtils;

import java.awt.*;
import java.util.BitSet;

import static org.lwjgl.glfw.GLFW.glfwGetTime;

public final class HierarchyPanel extends Panel {

    private final BitSet hierarchyBits = new BitSet();

    private final int hierarchyIntention = 18;

    private int selected = -1;

    private final EntityContext context;

    private double time = 0.0f;
    private double lastPress = 0.0f;

    private final DevelopmentStyle style;

    public HierarchyPanel(EntityContext context, DevelopmentStyle style) {
        this.context = context;
        this.style = style;
    }

    @Override
    public void init(Graphics gfx) {
    }

    @Override
    public void draw(Graphics gfx) {

        final double currentTime = glfwGetTime();
        final double dt = time>0.0 ? (float) (glfwGetTime() - time) : 1.0f/60.0f;
        time = currentTime;
        lastPress += dt;

        ImGui.pushID(ImGui.getID("Hierarchy-ID"));

        ImGui.begin("Hierarchy");

        ImVec2 cp = new ImVec2();
        ImGui.getCursorPos(cp);

        EntityComponentSystem ecs = context.getEcs();

        for (Entity entity : ecs.entities()) {
            if (entity.getParent() == null) {
                hierarchy(0, entity);
            }
        }

        ImGui.end();

        ImGui.popID();
    }

    private void hierarchy(int xoffset, Entity entity) {
        //TODO select correct texture
        ImGui.setCursorPosX(xoffset);

        if (beginHierarchyTreeNode(entity.getEntityIndex(), entity.getName(), style.getEntityIcon(entity), !entity.hasChildren())) {
            for (Entity child : entity.children()) {
                hierarchy(xoffset + hierarchyIntention, child);
            }
        }
    }

    private boolean beginHierarchyTreeNode(int id, String label, Texture2D texture, boolean leaf) {

        ImVec2 cp = ImGui.getCursorPos();
        ImVec2 region = ImGui.getContentRegionAvail();

        boolean isSelected = id == this.selected;
        Color selectedColor;

        if (isSelected) {
            ImGui.pushStyleColor(ImGuiCol.Button, style.getDarkColor());
            ImGui.pushStyleColor(ImGuiCol.ButtonHovered, style.getDarkColor());
            ImGui.pushStyleColor(ImGuiCol.ButtonActive, style.getDarkColor());
            selectedColor = style.getDarkColorAWT();
        } else {
            ImGui.pushStyleColor(ImGuiCol.Button, style.getBrightColor());
            ImGui.pushStyleColor(ImGuiCol.ButtonHovered, style.getBrightColor());
            ImGui.pushStyleColor(ImGuiCol.ButtonActive, style.getBrightColor());
            selectedColor = style.getMainColorAWT();
        }


        ImGui.setCursorPos(cp.x, cp.y);
        boolean pressed;
        if (isSelected) {
            pressed = ImGui.button(" ", region.x - cp.x, 20);
        } else {
            pressed = ImGui.invisibleButton(label, region.x - cp.x, 20);
        }

        ImGui.setCursorPos(cp.x + 40, cp.y+2);
        ImGui.text(label);

        if(!leaf){
            ImGui.setCursorPos(cp.x + 5, cp.y + 5);
            pressed |= ImGuiUtils.imageButton(hierarchyBits.get(id)?style.getOpenTreeIcon().getID():style.getClosedTreeIcon().getID(),
                    10, 10, selectedColor);
        }

        ImGui.setCursorPos(cp.x + 20, cp.y + 2);
        pressed |= ImGuiUtils.imageButton(texture.getID(), 16, 16, selectedColor);

        if (pressed) {
            if(selected == id && lastPress < 0.25f){
                hierarchyBits.flip(id);
            }
            selected = id;
            lastPress = 0;
        }

        ImGui.popStyleColor(3);

        ImGui.setCursorPos(cp.x, cp.y + 20);

        return hierarchyBits.get(id) && !leaf;
    }

    public Entity getSelectedEntity(){
        if(selected == -1)return null;
        return context.getEcs().getEntity(selected);
    }
}
