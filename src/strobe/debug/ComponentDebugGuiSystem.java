package strobe.debug;

import imgui.ImGui;
import marshal.MarshalComponent;
import marshal.MarshalEntity;
import marshal.MarshalStaticArray;
import marshal.MarshalSystem;
import org.joml.Vector3f;
import strobe.Strobe;

import java.lang.reflect.Field;

public class ComponentDebugGuiSystem extends MarshalSystem {


    private MarshalStaticArray<MarshalEntity> entities;

    @Override
    public void init() {
        entities = Strobe.getAllEntities();
        Strobe.addDebugGui(() -> {
            ImGui.begin("Component Debug ");
            try {
                for (MarshalEntity entity : entities) {
                    if (ImGui.treeNode(entity.toString())) {
                        for (MarshalComponent component : entity.getComponentIterator()) {
                            if (ImGui.treeNode(component.getClass().getSimpleName())) {
                                //component debug menu

                                for (Field field : component.getClass().getFields()) {
                                    if (ImGui.treeNodeEx(field.getName())) {
                                        ShowInDebug show_ind = field.getAnnotation(ShowInDebug.class);
                                        if (show_ind != null) {
                                            if (field.getType().equals(Vector3f.class)) {
                                                Vector3f field_vec3 = (Vector3f) field.get(component);
                                                float[] vec3 = new float[]{field_vec3.x, field_vec3.y, field_vec3.z};
                                                ImGui.inputFloat3("", vec3);
                                            }
                                        }
                                        SliderInDebug slider_ind = field.getAnnotation(SliderInDebug.class);
                                        if (slider_ind != null) {
                                            ImGui.pushItemWidth(-1);
                                            if (field.getType().equals(Vector3f.class)) {
                                                Vector3f field_vec3 = (Vector3f) field.get(component);
                                                float[] vec3 = new float[]{field_vec3.x, field_vec3.y, field_vec3.z};
                                                ImGui.sliderFloat3(component.toString() + field.getName(), vec3, Float.parseFloat(slider_ind.min()), Float.parseFloat(slider_ind.max()));
                                                field_vec3.set(vec3[0], vec3[1], vec3[2]);
                                            }
                                            ImGui.popItemWidth();
                                        }
                                        DragInDebug drag_ind = field.getAnnotation(DragInDebug.class);
                                        if (drag_ind != null) {
                                            ImGui.pushItemWidth(-1);
                                            if (field.getType().equals(Vector3f.class)) {
                                                Vector3f field_vec3 = (Vector3f) field.get(component);
                                                float[] vec3 = new float[]{field_vec3.x, field_vec3.y, field_vec3.z};
                                                ImGui.dragFloat3(component.toString() + field.toString(), vec3, Float.parseFloat(drag_ind.speed()));
                                                field_vec3.set(vec3[0], vec3[1], vec3[2]);
                                            }
                                            ImGui.popItemWidth();
                                        }
                                        ImGui.treePop();
                                    }
                                }
                                ImGui.treePop();
                            }
                        }
                        ImGui.treePop();
                    }
                }
            } catch (IllegalAccessException e) {
                e.printStackTrace();
            }
            ImGui.end();
        });
    }

    @Override
    public void update(float dt) {

    }

    @Override
    public void cleanUp() {

    }
}
