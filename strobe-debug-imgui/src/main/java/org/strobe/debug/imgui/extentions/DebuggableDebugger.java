package org.strobe.debug.imgui.extentions;

import imgui.ImColor;
import imgui.ImGui;
import imgui.flag.ImGuiCol;
import imgui.flag.ImGuiStyleVar;
import imgui.flag.ImGuiWindowFlags;
import org.joml.Vector2i;
import org.strobe.debug.DebugAnnotationProcessor;
import org.strobe.debug.Debuggable;
import org.strobe.debug.ProcessedDebuggableClass;
import org.strobe.debug.imgui.ImGuiDebugger;
import org.strobe.debug.imgui.TypeDebugger;
import org.strobe.gfx.Graphics;

import java.lang.reflect.Field;
import java.lang.reflect.Type;
import java.util.LinkedList;

public class DebuggableDebugger extends TypeDebugger {

    private final LinkedList<InstanceDebugger> instanceDebuggers = new LinkedList<>();

    public DebuggableDebugger() {
        newLineDebugger = true;
    }

    @Override
    public Vector2i calculateBoxDimension(Object object) {
        InstanceDebugger instanceDebugger = getInstanceDebugger(object);
        if (instanceDebugger == null) {
            instanceDebugger = new InstanceDebugger(object);
            addInstanceDebugger(instanceDebugger);
            ;
        }
        return new Vector2i(instanceDebugger.width, instanceDebugger.height);
    }

    @Override
    public Type[] getTypesToApply() {
        return new Type[]{Debuggable.class};
    }

    @Override
    public void debugType(Graphics gfx, Object obj, float cursorOffsetX, float cursorOffsetY) {
        InstanceDebugger instanceDebugger = getInstanceDebugger(obj);
        if (instanceDebugger == null) throw new IllegalStateException();
        instanceDebugger.debugInstance(gfx, cursorOffsetX, cursorOffsetY);
        removeInstanceDebugger(instanceDebugger);
    }


    private void addInstanceDebugger(InstanceDebugger instanceDebugger) {
        instanceDebuggers.add(instanceDebugger);
    }

    private InstanceDebugger getInstanceDebugger(Object object) {
        if (object == null) return null;
        for (InstanceDebugger instanceDebugger : instanceDebuggers) {
            if (instanceDebugger.debuggable == object) return instanceDebugger;
        }
        return null;
    }

    private void removeInstanceDebugger(InstanceDebugger instanceDebugger) {
        instanceDebuggers.remove(instanceDebugger);
    }

    private final class InstanceDebugger {

        private final Debuggable debuggable;
        private final ProcessedDebuggableClass parsedClass;
        private final int width;
        private final int height;

        public InstanceDebugger(Object object) {
            Debuggable debuggable = (Debuggable) object;
            this.debuggable = debuggable;
            //calculate height
            float height = 0;
            float width = 0;
            parsedClass = DebugAnnotationProcessor.process(debuggable);

            for (int i = 0; i < parsedClass.getFields().length; i++) {
                if (!parsedClass.getDisplay()[i]) continue;

                Field field = parsedClass.getFields()[i];
                if (!parsedClass.getAccessible()[i]) field.setAccessible(true);

                Object value = null;
                try {
                    value = field.get(debuggable);
                } catch (IllegalAccessException e) {
                    e.printStackTrace();
                }

                TypeDebugger typeDebugger = renderer.getTypeDebuggerForType(field.getType());
                typeDebugger.configureFor(value, field.getName());
                height = typeDebugger.getBoxHeight() + ImGuiDebugger.ATTRIB_BOX_PADDING_Y;
                width = Math.max(width, typeDebugger.getBoxWidth());

                if (!parsedClass.getAccessible()[i]) field.setAccessible(false);
            }
            height += ImGuiDebugger.ATTRIB_BOX_PADDING_Y;
            width += ImGuiDebugger.ATTRIB_BOX_PADDING_X * 2;
            this.width = (int) Math.ceil(width);
            this.height = (int) Math.ceil(height);
        }


        public void debugInstance(Graphics gfx, float cursorOffsetX, float cursorOffsetY) {
            float cursorX = ImGui.getCursorPosX();
            float cursorY = ImGui.getCursorPosY() + ImGuiDebugger.ATTRIB_BOX_PADDING_Y;

            for (int i = 0; i < parsedClass.getFields().length; i++) {
                if (!parsedClass.getDisplay()[i]) continue;
                Field field = parsedClass.getFields()[i];
                if (!parsedClass.getAccessible()[i]) field.setAccessible(true);

                Object value = null;
                try {
                    value = field.get(debuggable);
                } catch (IllegalAccessException e) {
                    e.printStackTrace();
                }
                fieldTitle(field.getName(), cursorX + 10, cursorY - 10);

                TypeDebugger typeDebugger = renderer.getTypeDebuggerForType(field.getType());
                typeDebugger.setWindowScroll(windowScrollX, windowScrollY);
                typeDebugger.setWindowSize(windowWidth, windowHeight);
                typeDebugger.configureFor(value, field.getName());
                ImGui.setCursorPos(cursorX, cursorY);
                if (windowScrollY + windowHeight > cursorY + cursorOffsetY + 2
                        && windowScrollY < cursorY + cursorOffsetY + typeDebugger.getBoxHeight() - 25) {
                    if (ImGui.beginChild(debuggable.toString() + field.toString(),
                            typeDebugger.getBoxWidth(), typeDebugger.getBoxHeight(), true,
                            ImGuiWindowFlags.NoScrollWithMouse | ImGuiWindowFlags.NoScrollbar)) {
                        typeDebugger.debugType(gfx, value, cursorOffsetX + cursorX, cursorOffsetY + cursorY);
                        ImGui.endChild();
                    }
                }
                fieldTitle(field.getName(), cursorX + 10, cursorY - 10);

                cursorY += typeDebugger.getBoxHeight() + ImGuiDebugger.ATTRIB_BOX_PADDING_Y;

                if (!parsedClass.getAccessible()[i]) field.setAccessible(false);
            }

        }

        private void fieldTitle(String title, float cursorX, float cursorY) {
            ImGui.setCursorPos(cursorX, cursorY);
            int attribNameBg = ImColor.intToColor(255, 255, 255, 255);
            int attribNameTxt = ImColor.intToColor(0, 0, 0, 255);
            ImGui.pushStyleColor(ImGuiCol.Button, attribNameBg);//button background
            ImGui.pushStyleColor(ImGuiCol.ButtonHovered, attribNameBg);//button background
            ImGui.pushStyleColor(ImGuiCol.ButtonActive, attribNameBg);//button background
            ImGui.pushStyleColor(ImGuiCol.Text, attribNameTxt);
            //push style var
            ImGui.pushStyleVar(ImGuiStyleVar.FrameRounding, 4);

            ImGui.button(title);

            //pop style var
            ImGui.popStyleVar(1);
            //pop style color
            ImGui.popStyleColor(4);
        }

    }

}
