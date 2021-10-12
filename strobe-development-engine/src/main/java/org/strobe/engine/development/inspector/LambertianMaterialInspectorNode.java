package org.strobe.engine.development.inspector;

import imgui.ImGui;
import imgui.ImVec2;
import org.joml.Vector3f;
import org.strobe.ecs.Component;
import org.strobe.ecs.Entity;
import org.strobe.ecs.context.renderer.materials.LambertianMaterial;
import org.strobe.engine.development.DevelopmentStyle;
import org.strobe.gfx.Graphics;

public class LambertianMaterialInspectorNode extends ComponentInspectorNode<LambertianMaterial>{

    @Override
    public Class[] getInspectorTargets() {
        return new Class[]{LambertianMaterial.class};
    }

    @Override
    protected void drawInspectorNode(Graphics gfx, DevelopmentStyle style, LambertianMaterial material, Entity entity) {
        ImVec2 cp = ImGui.getCursorPos();

        ImGui.setCursorPos(cp.x, cp.y+1);
        ImGui.bulletText("diffuse");

        float[] diffuse = new float[]{material.getDiffuseColor().x, material.getDiffuseColor().y, material.getDiffuseColor().z};
        ImGui.pushID("diffuse");
        ImGui.pushItemWidth(200);
        ImGui.setCursorPos(cp.x+100, cp.y);
        boolean confirm = ImGui.colorEdit3("", diffuse);
        Vector3f diffuseMod = new Vector3f(diffuse);
        if(confirm && !diffuseMod.equals(material.getDiffuseColor(), 0.0001f)){
            System.out.println("change");
            material.setDiffuseColor(diffuseMod);
        }
        ImGui.popItemWidth();
        ImGui.popID();
    }

}
