package org.strobe.debug.imgui.extentions.util;

import imgui.ImGui;
import org.joml.Vector2i;
import org.strobe.debug.imgui.TypeDebugger;
import org.strobe.gfx.Graphics;

import java.lang.reflect.Type;
import java.util.LinkedList;
import java.util.Map;

public class MapDebugger extends TypeDebugger {

    private final LinkedList<InstanceDebugger> instanceDebuggers = new LinkedList<>();

    @Override
    public void configureFor(Object object, String name) {
        InstanceDebugger instanceDebugger = getInstanceDebugger(object);
        if(instanceDebugger == null){
            instanceDebugger = new InstanceDebugger(object, name);
            addInstanceDebugger(instanceDebugger);
        }
        this.typeBoxHeight = instanceDebugger.height;
        this.typeBoxWidth = instanceDebugger.width;
        this.fieldName = name;
    }

    private void addInstanceDebugger(InstanceDebugger instanceDebugger){
        instanceDebuggers.add(instanceDebugger);
    }

    private InstanceDebugger getInstanceDebugger(Object object){
        if(object == null)return null;
        for(InstanceDebugger instanceDebugger : instanceDebuggers) {
            if(instanceDebugger.map == object)return instanceDebugger;
        }
        return null;
    }

    private void removeInstanceDebugger(InstanceDebugger instanceDebugger){
        instanceDebuggers.remove(instanceDebugger);
    }

    @Override
    public Vector2i calculateBoxDimension(Object object) {
        throw new UnsupportedOperationException();
    }

    @Override
    protected Type[] getTypesToApply() {
        return new Type[]{Map.class};
    }

    @Override
    public void debugType(Graphics gfx, Object value, float cursorOffsetX, float cursorOffsetY) {
        InstanceDebugger instanceDebugger = getInstanceDebugger(value);
        if(instanceDebugger == null)throw new IllegalStateException();
        instanceDebugger.debugInstance(gfx, cursorOffsetX, cursorOffsetY);
        removeInstanceDebugger(instanceDebugger);
    }

    private final class InstanceDebugger {

        private final Map map;
        private final String fieldName;
        private final int width;
        private final int height;

        public InstanceDebugger(Object object, String fieldName){
            map = (Map)object;
            this.fieldName = fieldName;
            width = 200;
            height = 50;
        }

        private void debugInstance(Graphics gfx, float cursorOffsetX, float cursorOffsetY){
            ImGui.setCursorPos(10, 50/2-ImGui.getFontSize()/2);
            ImGui.button("Map is to be implemented");
        }

    }
}
