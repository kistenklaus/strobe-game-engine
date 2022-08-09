package strobe;

import aspect.AspectLocation;
import aspect.AspectNode;
import aspect.AspectPass;
import marshal.*;

import java.util.List;

public final class Strobe {

    private static Strobe instance;

    private static Strobe getInstance() {
        if(instance==null)instance = new Strobe();
        return instance;
    }

    public static void createContext(){
        getInstance().strobe_context.addFlag(StrobeContext.DEBUG);
        getInstance().strobe_context.start();
    }

    public static void disposeContext(){
        getInstance().strobe_context.stop();
    }

    public static void addEntity(MarshalEntity entity){
        getInstance().strobe_context.addEntity(entity);
    }

    public static void removeEntity(MarshalEntity entity){
        getInstance().strobe_context.removeEntity(entity);
    }

    public static void addSystem(MarshalSystem system){
        getInstance().strobe_context.addSystem(system);
    }

    public static void removeSystem(MarshalSystem system){
        getInstance().strobe_context.removeSystem(system);
    }

    public static void addGraphicsSystem(MarshalSystem system){
        getInstance().strobe_context.addGraphicsSystem(system);
    }

    public static void removeGraphicsSystem(MarshalSystem system){
        getInstance().strobe_context.removeGraphicsSystem(system);
    }

    public static void addRenderNode(AspectNode node, AspectLocation location){
        getInstance().strobe_context.addRenderNode(node, location);
    }

    public static void removeRenderNode(AspectLocation location){
        getInstance().strobe_context.removeRenderNode(location);
    }

    public static void removeRenderNode(AspectNode node) {
        getInstance().strobe_context.removeRenderNode(AspectLocation.sis(node));
    }

    public static void addDebugGui(String gui_name, AspectPass gui_pass){
        getInstance().strobe_context.addDebugGui(gui_name, gui_pass);
    }

    public static void addDebugGui(AspectPass gui_pass){
        getInstance().strobe_context.addDebugGui(gui_pass.toString(), gui_pass);
    }

    public static void removeDebugGui(String gui_name){
        getInstance().strobe_context.removeDebugGui(gui_name);
    }

    public static void removeDebugGui(AspectPass gui_pass){
        getInstance().strobe_context.removeDebugGui(gui_pass);
    }

    public static List<MarshalEntity> getEntities(MarshalFamily.MarshalFamilyIdentifier family){
        return getInstance().strobe_context.getEntities(family);
    }

    public static MarshalStaticArray getAllEntities(){
        return Marshal.getAllEntities();
    }

    private final StrobeContext strobe_context;

    private Strobe(){
        this.strobe_context = new StrobeContext();
    }

}
