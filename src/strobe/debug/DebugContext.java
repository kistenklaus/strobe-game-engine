package strobe.debug;

import aspect.Aspect;
import aspect.AspectLocation;
import aspect.AspectNode;

import aspect.AspectPass;
import strobe.graphics.Window;

import java.util.concurrent.atomic.AtomicReference;

public class DebugContext {

    private GuiLayer layer;
    private AspectNode debug_root;
    private AspectPass init_pass = null, dispose_pass = null;

    public DebugContext(AtomicReference<Window> window_ref) {
        debug_root = Aspect.createNode("debug-node-root");

        debug_root.setInitPass(() -> {
            layer = new GuiLayer(window_ref.get());
            if (init_pass != null) init_pass.execute();
        });
        debug_root.setDownPass(() -> {
            layer.startFrame();
            if (dispose_pass != null) dispose_pass.execute();
        });
        debug_root.setUpPass(() -> {
            layer.endFrame();
        });
        debug_root.setDisposePass(() -> {
            layer.dispose();
        });
    }

    public AspectNode getDebugRoot() {
        return this.debug_root;
    }

    public void addGui(AspectPass gui_pass) {
        addGui(gui_pass.toString(), gui_pass);
    }

    public void addGui(String gui_name, AspectPass gui_pass) {
        AspectNode gui_node = Aspect.createNode("debug-node-" + gui_name);
        gui_node.setDownPass(gui_pass);
        debug_root.appendChild(gui_node);
    }

    public void removeGui(String gui_name) {
        Aspect.removeNode(AspectLocation.schildOf(AspectLocation.sis("debug-node-root")).is("debug-node-" + gui_name));
    }

    public void onInit(AspectPass init_pass) {
        this.init_pass = init_pass;
    }


    public void onDispose(AspectPass dispose_pass) {
        this.dispose_pass = dispose_pass;
    }

}
