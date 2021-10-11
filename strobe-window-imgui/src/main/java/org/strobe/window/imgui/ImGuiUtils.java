package org.strobe.window.imgui;

import imgui.ImGui;
import imgui.ImVec2;
import imgui.flag.ImGuiCol;

import java.awt.*;

public final class ImGuiUtils {

    public static boolean imageButton(int id, float width, float height, Color bg, Color tint) {
        return ImGui.imageButton(id, width, height, 0, 1, 1, 0, 0,
                bg.getRed() / 255f, bg.getGreen() / 255f, bg.getBlue() / 255f, bg.getAlpha() / 255f,
                tint.getRed() / 255f, tint.getGreen() / 255f, tint.getBlue() / 255f, tint.getAlpha() / 255f);
    }

    public static boolean imageButton(int id, float width, float height, Color bg) {
        return ImGui.imageButton(id, width, height, 0, 1, 1, 0, 0,
                bg.getRed() / 255f, bg.getGreen() / 255f, bg.getBlue() / 255f, bg.getAlpha() / 255f);
    }

    public static void image(int id, float width, float height) {
        ImGui.image(id, width, height, 0, 1, 1, 0);
    }



    public static boolean iconTreeNodeButton(int id, String label, int width, int height, boolean open,
                                             boolean highlight, int highlightColor,
                                             int openTexId, int closedTexId, int iconTexId){
        ImVec2 region = ImGui.getContentRegionAvail();
        ImVec2 cp = ImGui.getCursorPos();

        ImGui.pushID(id);

        ImGui.setCursorPos(cp.x, cp.y);
        boolean pressed;
        if(highlight){
            ImGui.pushStyleColor(ImGuiCol.Button, highlightColor);
            ImGui.pushStyleColor(ImGuiCol.ButtonHovered, highlightColor);
            ImGui.pushStyleColor(ImGuiCol.ButtonActive, highlightColor);
            pressed = ImGui.button("", width==-1?region.x:width, height);
            ImGui.popStyleColor(3);
        }else{
            pressed = ImGui.invisibleButton("", width==-1?region.x:width, height);
        }

        ImGui.setCursorPos(cp.x+height/4, cp.y+height/4);
        ImGuiUtils.image(open?openTexId:closedTexId, height/2,height/2);

        ImGui.setCursorPos(cp.x + height*1.1f, cp.y+height*0.1f);
        ImGuiUtils.image(iconTexId, height*0.8f,height*0.8f);


        ImGui.setCursorPos(cp.x + height*2.2f, cp.y+height/2-ImGui.getFontSize()/2f);
        ImGui.text(label);

        ImGui.setCursorPos(cp.x, cp.y + height+3);
        ImGui.popID();


        return pressed;
    }

}
