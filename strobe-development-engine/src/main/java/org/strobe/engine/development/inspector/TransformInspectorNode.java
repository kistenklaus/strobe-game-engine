package org.strobe.engine.development.inspector;

import imgui.ImGui;
import imgui.ImVec2;
import imgui.flag.ImGuiInputTextFlags;
import org.joml.Quaternionf;
import org.joml.Vector3f;
import org.strobe.ecs.Entity;
import org.strobe.ecs.context.renderer.transform.Transform;
import org.strobe.engine.development.DevelopmentStyle;
import org.strobe.gfx.Graphics;

public final class TransformInspectorNode extends ComponentInspectorNode<Transform> {


    @Override
    @SuppressWarnings("unchecked")
    public Class<? extends Transform>[] getInspectorTargets() {
        return new Class[]{Transform.class};
    }

    @Override
    public void drawInspectorNode(Graphics gfx, DevelopmentStyle style, Transform transform, Entity entity) {

        ImVec2 cp = ImGui.getCursorPos();

        ImGui.setCursorPos(cp.x, cp.y+1);
        ImGui.bulletText("position");
        float[] pos = new float[]{transform.getX(), transform.getY(), transform.getZ()};
        ImGui.pushID("position");
        ImGui.pushItemWidth(200);
        ImGui.setCursorPos(cp.x + 100, cp.y);
        boolean confirmed = ImGui.inputFloat3("", pos, "%.4g",
                 ImGuiInputTextFlags.EnterReturnsTrue);
        ImGui.popItemWidth();
        ImGui.popID();
        if (confirmed) transform.setPosition(pos[0], pos[1], pos[2]);



        ImGui.setCursorPos(cp.x, cp.y+26);
        ImGui.bulletText("scale");
        float[] scale = new float[]{transform.getSX(), transform.getSY(), transform.getSZ()};
        ImGui.pushID("scale");
        ImGui.pushItemWidth(200);
        ImGui.setCursorPos(cp.x+100, cp.y+25);
        confirmed = ImGui.inputFloat3("", scale, "%.4g", ImGuiInputTextFlags.EnterReturnsTrue);
        ImGui.popID();
        if(confirmed) transform.setScale(scale[0], scale[1], scale[2]);


        ImGui.setCursorPos(cp.x, cp.y + 51);
        ImGui.bulletText("rotation");
        Vector3f euler = transform.getEulerRotation();
        float[] rot = new float[]{euler.x, euler.y, euler.z};
        ImGui.pushID("rotation");
        ImGui.pushItemWidth(200);
        ImGui.setCursorPos(cp.x+100, cp.y + 50);
        confirmed = ImGui.inputFloat3("", rot, "%.4g", ImGuiInputTextFlags.EnterReturnsTrue);
        ImGui.popID();
        if(confirmed){
            double cx = Math.cos(rot[0] * 0.5f);
            double sx = Math.sin(rot[0] * 0.5f);
            double cy = Math.cos(rot[1] * 0.5f);
            double sy = Math.sin(rot[1] * 0.5f);
            double cz = Math.cos(rot[2] * 0.5f);
            double sz = Math.sin(rot[2] * 0.5f);
            Quaternionf orientation = new Quaternionf(
                    cz * cy * cx + sz * sy * sx,
                    sz * cy * cx - cz * sy * sx,
                    cz * sy * cx + sz * cy * sx,
                    cz * cy * sx - sz * sy * cx
            );
            transform.setOrientation(orientation);
        }




    }
}
