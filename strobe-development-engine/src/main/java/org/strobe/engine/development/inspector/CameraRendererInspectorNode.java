package org.strobe.engine.development.inspector;

import imgui.ImGui;
import imgui.ImVec2;
import imgui.type.ImBoolean;
import imgui.type.ImInt;
import org.strobe.ecs.Entity;
import org.strobe.ecs.context.renderer.camera.CameraRenderer;
import org.strobe.engine.development.DevelopmentStyle;
import org.strobe.gfx.Graphics;

public final class CameraRendererInspectorNode extends ComponentInspectorNode<CameraRenderer> {

    private static final String[] SCALING_OPT = new String[]{"LINEAR_FILTERING", "NEAREST_FILTERING"};

    @Override
    public Class<? extends CameraRenderer>[] getInspectorTargets() {
        return new Class[]{CameraRenderer.class};
    }

    @Override
    protected void drawInspectorNode(Graphics gfx, DevelopmentStyle style, CameraRenderer cameraRenderer, Entity entity) {
        ImVec2 cp = ImGui.getCursorPos();

        ImGui.setCursorPos(cp.x, cp.y + 1);
        ImGui.bulletText("resolution");

        ImGui.pushID("resolution");
        int[] res = new int[]{cameraRenderer.getHorResolution(), cameraRenderer.getVerResolution()};
        ImGui.pushItemWidth(200);
        ImGui.setCursorPos(cp.x + 100, cp.y);
        boolean confirm = ImGui.inputInt2("", res);
        if (res[0] > 8192) res[0] = 8192;
        if (res[1] > 8192) res[1] = 8192;
        ImGui.popID();
        if (confirm && res[0] != cameraRenderer.getHorResolution() || res[1] != cameraRenderer.getVerResolution()) {
            cameraRenderer.setResolution(res[0], res[1]);
        }


        ImGui.setCursorPos(cp.x, cp.y + 26);
        ImGui.bulletText("scaling");

        ImGui.pushID("scaling");
        int mode = cameraRenderer.isLinearScalingEnabled() ? 0 : 1;
        ImInt modeMod = new ImInt(mode);
        ImGui.setCursorPos(cp.x + 100, cp.y + 25);
        ImGui.pushItemWidth(200);
        ImGui.combo("", modeMod, SCALING_OPT, 10);
        if (mode != modeMod.get()) {
            if (modeMod.get() == 0) cameraRenderer.enableLinearScaling();
            else if (modeMod.get() == 1) cameraRenderer.disableLinearScaling();
        }
        ImGui.popItemWidth();
        ImGui.popID();


        ImGui.setCursorPos(cp.x, cp.y + 51);
        ImGui.bulletText("FXAA");

        ImGui.pushID("FXAA");
        ImBoolean fxaa = new ImBoolean(cameraRenderer.isFxaaEnabled());
        ImGui.setCursorPos(cp.x + 100, cp.y + 50);
        if (ImGui.checkbox("", fxaa)) {
            if (fxaa.get() && !cameraRenderer.isFxaaEnabled()) cameraRenderer.enableFXAA();
            else if (!fxaa.get() && cameraRenderer.isFxaaEnabled()) cameraRenderer.disableFXAA();
        }
        ImGui.popID();

        ImGui.setCursorPos(cp.x, cp.y + 76);
        ImGui.bulletText("shadows");

        ImGui.pushID("shadows");
        ImGui.setCursorPos(cp.x + 100, cp.y + 75);
        ImBoolean shadows = new ImBoolean(cameraRenderer.isRenderingShadows());
        if (ImGui.checkbox("", shadows)) {
            if (shadows.get() && !cameraRenderer.isRenderingShadows()) cameraRenderer.enableShadows();
            else if (!shadows.get() && cameraRenderer.isRenderingShadows()) cameraRenderer.disableShadows();
        }
        ImGui.popID();

    }
}
