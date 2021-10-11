package org.strobe.engine.development.inspector;

import imgui.ImGui;
import imgui.ImVec2;
import org.strobe.ecs.Entity;
import org.strobe.ecs.context.renderer.camera.PerspectiveLens;
import org.strobe.engine.development.DevelopmentStyle;
import org.strobe.gfx.Graphics;

public final class PerspectiveLensInspectorNode extends ComponentInspectorNode<PerspectiveLens> {

    @Override
    public Class<? extends PerspectiveLens>[] getInspectorTargets() {
        return new Class[]{PerspectiveLens.class};
    }

    @Override
    protected void drawInspectorNode(Graphics gfx, DevelopmentStyle style, PerspectiveLens lens, Entity entity) {
        ImVec2 cp = ImGui.getCursorPos();


        ImGui.setCursorPos(cp.x, cp.y + 1);
        ImGui.bulletText("FOV");

        float[] fov = new float[]{(float) Math.toDegrees(lens.getFov())};
        ImGui.pushID("FOV");
        ImGui.pushItemWidth(200);
        ImGui.setCursorPos(cp.x + 100, cp.y);
        ImGui.dragFloat("", fov, 1f, 0f, 170);
        ImGui.popID();
        if (Math.abs(fov[0] - Math.toDegrees(lens.getFov())) > 0.1f) {
            lens.setFov((float) Math.toRadians(fov[0]));
        }

        ImGui.setCursorPos(cp.x, cp.y + 26);
        ImGui.bulletText("aspect");

        float[] aspect = new float[]{lens.getAspect()};
        ImGui.pushID("aspect");
        ImGui.pushItemWidth(200);
        ImGui.setCursorPos(cp.x + 100, cp.y + 25);
        ImGui.dragFloat("",  aspect, 0.01f, 0.01f, 4f);
        ImGui.popID();
        if(Math.abs(aspect[0] - lens.getAspect()) > 0.001f)
            lens.setAspect(aspect[0]);


        ImGui.setCursorPos(cp.x, cp.y+51);
        ImGui.bulletText("near");

        float[] near = new float[]{lens.getNear()};
        ImGui.pushID("near");
        ImGui.pushItemWidth(200);
        ImGui.setCursorPos(cp.x+100, cp.y+50);
        ImGui.dragFloat("", near, 0.01f, 0.001f, 10000f);
        ImGui.popID();
        if(Math.abs(near[0] - lens.getNear()) > 0.001f){
            lens.setNear(near[0]);
        }

        ImGui.setCursorPos(cp.x, cp.y + 76);
        ImGui.bulletText("far");

        float[] far = new float[]{lens.getFar()};
        ImGui.pushID("far");
        ImGui.pushItemWidth(200);
        ImGui.setCursorPos(cp.x + 100, cp.y + 75);
        ImGui.dragFloat("", far, 0.01f, 0.001f, 1000f);
        ImGui.popID();
        if(Math.abs(far[0] - lens.getFar()) > 0.001f){
            lens.setFar(far[0]);
        }

        ImGui.setCursorPos(cp.x, cp.y + 102);
    }
}
