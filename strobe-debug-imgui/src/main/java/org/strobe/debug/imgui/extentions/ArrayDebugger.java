package org.strobe.debug.imgui.extentions;

import imgui.ImColor;
import imgui.ImGui;
import imgui.flag.ImGuiCol;
import imgui.flag.ImGuiStyleVar;
import imgui.flag.ImGuiWindowFlags;
import org.joml.Vector2i;
import org.strobe.debug.imgui.ImGuiDebugger;
import org.strobe.debug.imgui.TypeDebugger;
import org.strobe.gfx.Graphics;
import org.strobe.gfx.opengl.bindables.framebuffer.Framebuffer;
import org.strobe.gfx.opengl.bindables.framebuffer.FramebufferAttachment;

import java.lang.reflect.Type;
import java.util.*;

public class ArrayDebugger extends TypeDebugger {

    private final LinkedList<InstanceArrayDebugger> instanceDebuggers = new LinkedList<>();

    public ArrayDebugger() {
        newLineDebugger = true;
    }

    @Override
    public void configureFor(Object object, String name) {
        InstanceArrayDebugger instanceDebugger = getCollectionDebugger(object);
        if (instanceDebugger == null) {
            instanceDebugger = new InstanceArrayDebugger((Object[]) object, name);
            addInstanceDebugger(instanceDebugger);
        }
        typeBoxWidth = instanceDebugger.boxWidth;
        typeBoxHeight = instanceDebugger.boxHeight;
        this.fieldName = name;
    }

    @Override
    public Vector2i calculateBoxDimension(Object object) {
        throw new UnsupportedOperationException();
    }

    @Override
    public Type[] getTypesToApply() {
        return new Type[]{Framebuffer[].class, FramebufferAttachment[].class,
                int[].class, short[].class, long[].class,
                float[].class, double[].class};
    }

    @Override
    public void debugType(Graphics gfx, Object value, float cursorOffsetX, float cursorOffsetY) {
        InstanceArrayDebugger instanceDebugger = getCollectionDebugger(value);
        if (instanceDebugger == null) throw new IllegalStateException();
        instanceDebugger.debugType(gfx, cursorOffsetX, cursorOffsetY);
        removeCollectionDebugger(instanceDebugger);

    }

    private void addInstanceDebugger(InstanceArrayDebugger instanceDebugger) {
        instanceDebuggers.add(instanceDebugger);
    }

    private InstanceArrayDebugger getCollectionDebugger(Object object) {
        for (InstanceArrayDebugger debugger : instanceDebuggers) {
            if (debugger.array == object) return debugger;
        }
        return null;
    }

    private void removeCollectionDebugger(InstanceArrayDebugger debugger) {
        instanceDebuggers.remove(debugger);
    }

    private final class InstanceArrayDebugger {

        private final Object[] array;
        private final int boxWidth;
        private final int boxHeight;
        private final List<TypeDebugger> typeDebuggers = new ArrayList<>();
        private final String fieldName;

        private InstanceArrayDebugger(Object[] array, String fieldName) {
            this.array = array;
            this.fieldName = fieldName;

            float width = 310;
            float height = 0;
            int i=0;
            for (Object obj : array) {
                TypeDebugger typeDebugger = renderer.getTypeDebuggerForType(obj.getClass());
                typeDebugger.configureFor(obj, fieldName+"["+(i++)+"]");
                width = Math.max(width, typeDebugger.getBoxWidth());
                height += typeDebugger.getBoxHeight();
                typeDebuggers.add(typeDebugger);
            }
            height += ImGuiDebugger.ATTRIB_BOX_PADDING_Y*2 + ((array.length-1) * ImGuiDebugger.ATTRIB_BOX_PADDING_Y);
            width += ImGuiDebugger.ATTRIB_BOX_PADDING_X * 2;
            height = Math.max(height, 50);

            boxWidth = (int) Math.ceil(width);
            boxHeight = (int) Math.ceil(height);
        }

        public void debugType(Graphics gfx, float cursorOffsetX, float cursorOffsetY) {
            Iterator<Object> arrayIt = Arrays.asList(array).iterator();
            Iterator<TypeDebugger> debuggerIt = typeDebuggers.iterator();
            float cursorX = ImGui.getCursorPosX();
            float cursorY = ImGui.getCursorPosY()+ImGuiDebugger.ATTRIB_BOX_PADDING_Y;
            int i=0;
            while (arrayIt.hasNext() && debuggerIt.hasNext()) {
                Object o = arrayIt.next();
                String fieldName = "-"+ this.fieldName + "[" +i + "] : "+o.getClass().getSimpleName();
                fieldTitle(fieldName,cursorX +10 , cursorY - 10);
                TypeDebugger debugger = debuggerIt.next();
                debugger.setWindowSize(windowWidth, windowHeight);
                debugger.setWindowScroll(windowScrollX, windowScrollY);
                debugger.configureFor(o, fieldName);
                ImGui.setCursorPos(cursorX, cursorY);
                if (windowScrollY + windowHeight > cursorY + cursorOffsetY + 2
                        && windowScrollY < cursorY + cursorOffsetY + debugger.getBoxHeight() - 25) {
                    if (ImGui.beginChild("collectionDebugger:" + array.toString() + o.toString(),
                            debugger.getBoxWidth(), debugger.getBoxHeight(),
                            true, ImGuiWindowFlags.NoScrollbar | ImGuiWindowFlags.NoScrollWithMouse)) {
                        debugger.debugType(gfx, o, cursorOffsetX + cursorX, cursorOffsetY + cursorY);
                        ImGui.endChild();
                    }
                }
                fieldTitle(fieldName, cursorX +10 , cursorY - 10);
                cursorY += debugger.getBoxHeight() + ImGuiDebugger.ATTRIB_BOX_PADDING_Y;
                i++;
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
