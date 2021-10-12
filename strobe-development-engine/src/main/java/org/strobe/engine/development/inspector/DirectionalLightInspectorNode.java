package org.strobe.engine.development.inspector;

import imgui.ImGui;
import imgui.ImVec2;
import org.joml.Vector3f;
import org.strobe.ecs.Entity;
import org.strobe.ecs.context.renderer.light.DirectionalLight;
import org.strobe.engine.development.DevelopmentStyle;
import org.strobe.gfx.Graphics;

public final class DirectionalLightInspectorNode extends ComponentInspectorNode<DirectionalLight> {
    @Override
    public Class<? extends DirectionalLight>[] getInspectorTargets() {
        return new Class[]{DirectionalLight.class};
    }

    @Override
    protected void drawInspectorNode(Graphics gfx, DevelopmentStyle style, DirectionalLight light, Entity entity) {

        ImVec2 cp = ImGui.getCursorPos();

        ImGui.setCursorPos(cp.x, cp.y + 1);
        ImGui.bulletText("ambient");

        float[] ambient = new float[]{light.getAmbientColor().x, light.getAmbientColor().y, light.getAmbientColor().z};
        ImGui.pushID("ambient");
        ImGui.setCursorPos(cp.x + 100, cp.y);
        ImGui.pushItemWidth(200);
        ImGui.colorEdit3("", ambient);
        ImGui.popID();
        ImGui.popItemWidth();
        Vector3f ambientMod = new Vector3f(ambient);
        if (!ambientMod.equals(light.getAmbientColor(), 0.01f))
            light.setAmbientColor(ambientMod);

        ImGui.setCursorPos(cp.x, cp.y + 26);
        ImGui.bulletText("diffuse");

        float[] diffuse = new float[]{light.getDiffuseColor().x, light.getDiffuseColor().y, light.getDiffuseColor().z};
        ImGui.pushID("diffuse");
        ImGui.setCursorPos(cp.x + 100, cp.y + 25);
        ImGui.pushItemWidth(200);
        ImGui.colorEdit3("", diffuse);
        ImGui.popID();
        ImGui.popItemWidth();

        Vector3f diffuseMod = new Vector3f(diffuse);
        if(!diffuseMod.equals(light.getDiffuseColor(), 0.01f))
            light.setDiffuseColor(diffuseMod);


        ImGui.setCursorPos(cp.x, cp.y+51);
        ImGui.bulletText("specular");

        float[] specular = new float[]{light.getSpecularColor().x, light.getSpecularColor().y, light.getSpecularColor().z};
        ImGui.pushID("specular");
        ImGui.setCursorPos(cp.x+100, cp.y+50);
        ImGui.pushItemWidth(200);
        ImGui.colorEdit3("", specular);
        ImGui.popItemWidth();
        ImGui.popID();
        Vector3f specularMod = new Vector3f(specular);
        if(!specularMod.equals(light.getSpecularColor(), 0.01f))
            light.setSpecularColor(specularMod);

        ImGui.setCursorPos(cp.x, cp.y + 77);
    }
}
