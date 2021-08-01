package org.strobe.debug.imgui;

import imgui.ImColor;
import imgui.ImGui;
import imgui.ImVec2;
import imgui.flag.ImGuiCol;
import imgui.flag.ImGuiStyleVar;
import imgui.flag.ImGuiTreeNodeFlags;
import imgui.flag.ImGuiWindowFlags;
import org.strobe.debug.DebugAnnotationProcessor;
import org.strobe.debug.Debuggable;
import org.strobe.debug.Debugger;
import org.strobe.debug.ProcessedDebuggableClass;
import org.strobe.debug.imgui.extentions.ObjectDebugger;
import org.strobe.gfx.Graphics;

import java.awt.*;
import java.lang.reflect.Field;
import java.lang.reflect.Type;
import java.util.*;

public class ImGuiDebugger extends Debugger {

    private static final String WINDOW_TITLE = "ImGUI-DebugWindow";

    public static final float ATTRIB_BOX_PADDING_X = 10;
    public static final float ATTRIB_BOX_PADDING_Y = 20;

    private static final int ThemeColor = new Color(21, 77, 110).getRGB();
    private static final int ThemeColorDark = new Color(13, 61, 89).getRGB();
    private static final int ThemeColorLight = new Color(33, 99, 138).getRGB();

    private final ArrayList<Debuggable> queue = new ArrayList<>();
    private final ImGuiLayer layer;

    private final HashSet<Debuggable> openDebuggable = new HashSet<>();

    private final TypeDebugger[] typeDebuggers;
    private final HashMap<Type, Integer> typeIndices = new HashMap<>();

    private final TypeDebugger defaultDebugger;
    private boolean requestFocus = false;

    private final Graphics gfx;

    public ImGuiDebugger(Graphics gfx) {
        this.gfx = gfx;
        layer = new ImGuiLayer(gfx.getWindow());
        ImGui.getStyle().setButtonTextAlign(0.5f, 0f);
        ImGui.getStyle().setChildRounding(2);
        ImGui.getStyle().setWindowMinSize(300, 500);

        ImVec2 temp = new ImVec2();
        ImGui.calcTextSize(temp, "000000000");   //for scientific notation;

        ServiceLoader<TypeDebugger> service = ServiceLoader.load(TypeDebugger.class);
        
        int i=0;
        typeDebuggers = new TypeDebugger[(int) service.stream().count()];
        TypeDebugger tempDebugger = null;
        for(TypeDebugger debugger : service){
            typeDebuggers[i++] = debugger;
            if(debugger.getClass().equals(ObjectDebugger.class))tempDebugger = debugger;
        }
        this.defaultDebugger = tempDebugger;
        if(this.defaultDebugger == null)throw new IllegalStateException("no default debugger defined!");
        for(int j=0;j<typeDebuggers.length;j++){
            for(Type type : typeDebuggers[j].getTypesToApply()){
                typeIndices.put(type, j);
            }
        }
        //link typeDebugger
        for(TypeDebugger typeDebugger : typeDebuggers){
            typeDebugger.link(this);
        }
    }

    @Override
    public void dispose() {
        layer.dispose();
    }

    @Override
    public void debug(Debuggable debuggable) {
        //parseDebuggable
        queue.add(debuggable);
    }

    @Override
    public void render() {
        layer.startFrame();
        ImGui.begin(WINDOW_TITLE);

        if (ImGui.collapsingHeader("Debuggables")) {
            ImGui.beginGroup();
            for (Debuggable debuggable : queue) {
                ImGui.pushStyleColor(ImGuiCol.Header, ThemeColor);
                ImGui.pushStyleColor(ImGuiCol.HeaderActive, ThemeColorLight);
                ImGui.pushStyleColor(ImGuiCol.HeaderHovered, ThemeColorDark);
                int flag = ImGuiTreeNodeFlags.SpanAvailWidth;
                if (openDebuggable.contains(debuggable)) flag |= ImGuiTreeNodeFlags.Selected;
                boolean open = ImGui.treeNodeEx(debuggable.hashCode(), flag, debuggable.getClass().getSimpleName());
                if (ImGui.isItemClicked()) {
                    if (!openDebuggable.remove(debuggable)) openDebuggable.add(debuggable);
                }
                if (open) {
                    renderDebuggable(gfx, debuggable);
                    ImGui.separator();
                    ImGui.treePop();
                }
                ImGui.popStyleColor();
                ImGui.popStyleColor();
                ImGui.popStyleColor();
            }
            ImGui.endGroup();

            ImGui.dummy(0, 10);
            ImGui.separator();
        }

        queue.clear();
        ImGui.end();
        layer.endFrame();
    }

    public void renderDebuggable(Graphics gfx, Debuggable debuggable) {
        ProcessedDebuggableClass parsedClass = DebugAnnotationProcessor.process(debuggable);
        float top = ImGui.getCursorPosY();
        float left = ImGui.getCursorPosX();
        float columnWidth = ImGui.getColumnWidth() + left;

        float currentRowHeight = 0;

        float cursorX = left;
        float cursorY = top + ATTRIB_BOX_PADDING_Y;

        for (int i = 0; i < parsedClass.getFields().length; i++) {
            if (!parsedClass.getDisplay()[i]) continue;
            Field field = parsedClass.getFields()[i];
            Class type = field.getType();


            TypeDebugger typeDebugger = getTypeDebuggerForType(type);

            String attribName = field.getName();
            boolean readOnly = parsedClass.getReadOnly()[i];
            if (!parsedClass.getAccessible()[i]) field.setAccessible(true);

            typeDebugger.setWindowScroll(ImGui.getScrollX(), ImGui.getScrollY());
            typeDebugger.setWindowSize(ImGui.getWindowWidth(), ImGui.getWindowHeight());
            try {
                //configure typeDebugger
                typeDebugger.configureFor(field.get(debuggable), field.getName());
            } catch (IllegalAccessException e) {
                e.printStackTrace();
            }

            //check if the box fits in the column
            if (typeDebugger.newLineDebugger || columnWidth - cursorX <= typeDebugger.getBoxWidth()) {
                //new Line
                cursorX = left;
                cursorY += currentRowHeight + ATTRIB_BOX_PADDING_Y;
                currentRowHeight = typeDebugger.getBoxHeight();
            }
            currentRowHeight = Math.max(currentRowHeight, typeDebugger.getBoxHeight());

            ImGui.setCursorPos(cursorX, cursorY);

            if (ImGui.getScrollY()+ ImGui.getWindowHeight() > cursorY  + 2 && ImGui.getScrollY() < cursorY + typeDebugger.getBoxHeight() - 25) {
                float cX = ImGui.getCursorPosX();
                float cY = ImGui.getCursorPosY();
                if (ImGui.beginChild(debuggable.getClass() + "@" + debuggable.hashCode() + attribName, typeDebugger.getBoxWidth(), typeDebugger.getBoxHeight(), true, ImGuiWindowFlags.NoScrollbar | ImGuiWindowFlags.NoScrollWithMouse)) {
                    try {
                        typeDebugger.debugType(gfx, field.get(debuggable), cX, cY);
                    } catch (IllegalAccessException e) {
                        e.printStackTrace();
                    }
                    ImGui.endChild();
                }
            }


            ImGui.setCursorPos(cursorX + 10, cursorY - 10);
            //push style color
            int attribNameBg = ImColor.intToColor(255, 255, 255, 255);
            ImGui.pushStyleColor(ImGuiCol.Button, attribNameBg);//button background
            ImGui.pushStyleColor(ImGuiCol.ButtonHovered, attribNameBg);//button background
            ImGui.pushStyleColor(ImGuiCol.ButtonActive, attribNameBg);//button background
            int attribNameTxt = ImColor.intToColor(0, 0, 0, 255);
            ImGui.pushStyleColor(ImGuiCol.Text, attribNameTxt);
            //push style var
            ImGui.pushStyleVar(ImGuiStyleVar.FrameRounding, 5);

            //draw attrib title
            ImGui.button(attribName);
            //pop style var
            ImGui.popStyleVar(1);
            //pop style color
            ImGui.popStyleColor(4);

            //update cursorX position.
            cursorX += typeDebugger.getBoxWidth() + ATTRIB_BOX_PADDING_X;

            if (!parsedClass.getAccessible()[i]) field.setAccessible(false);
        }
        cursorY += currentRowHeight + ATTRIB_BOX_PADDING_Y;
        ImGui.setCursorPos(left, cursorY);
    }

    public TypeDebugger getTypeDebuggerForType(Class type){
        TypeDebugger typeDebugger = getTypeDebuggerForTypeRec(type);
        if(typeDebugger == null)return defaultDebugger;
        return typeDebugger;
    }

    private TypeDebugger getTypeDebuggerForTypeRec(Class type){
        if(type == null)return null;
        if(type == Object.class)return null;
        Integer index = typeIndices.get(type);
        if(index == null){
            //check interfaces
            Class[] interfaces = type.getInterfaces();
            for(Class iface : interfaces){
                TypeDebugger typeDebugger = getTypeDebuggerForTypeRec(iface);
                if(typeDebugger != null)return typeDebugger;
            }
            //check superclass
            return getTypeDebuggerForTypeRec(type.getSuperclass());
        }
        return typeDebuggers[index];
    }

}
