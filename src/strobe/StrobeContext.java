package strobe;

import aspect.*;
import marshal.*;
import strobe.debug.DebugContext;
import strobe.graphics.*;

import java.util.List;
import java.util.Scanner;
import java.util.concurrent.atomic.AtomicReference;

import static org.lwjgl.opengl.GL11.*;
import static org.lwjgl.opengl.GL15.*;

public class StrobeContext {

    private AspectContext graphics;
    private MarshalContext ecs, graphics_ecs;
    private DebugContext debug_gui;

    private boolean debug_flag = false;

    private boolean graphics_initialized = false, ecs_initialized = false,
            graphics_ecs_initialized = false, debug_gui_initialized = false;

    private void init() {

    }

    private void dispose() {
        System.out.println("marshal stoped");
        Aspect.stop();

    }

    public void stop() {
        Marshal.stop();
    }

    public void start() {
        ecs = createLogicEcs();
        AtomicReference<Window> window = new AtomicReference<>();
        graphics = createGraphicsContext(window);
        if (debug_flag) debug_gui = setupDebugContext(graphics, window);
        graphics_ecs = createGraphicsEcs(graphics);
        Marshal.start();
        Aspect.start(graphics);

        //wait for graphics / graphics_ecs / ecs / debug_graphics
        synchronized (this) {
            try {
                while (!graphics_initialized || !graphics_ecs_initialized || !ecs_initialized || (!debug_gui_initialized && debug_flag)) {
                    wait();
                }


            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
        System.out.println("strobe started");
    }

    private MarshalContext createLogicEcs() {
        MarshalContext logic_context = new MarshalContext();

        logic_context.on(Marshal.CONTEXT_INIT, () -> {
            System.out.println("marshal started");
            init();
            synchronized (StrobeContext.this) {
                ecs_initialized = true;
                StrobeContext.this.notify();
            }
        });
        logic_context.on(Marshal.CONTEXT_DISPOSED, () -> dispose());
        Marshal.addContext(logic_context);
        return logic_context;
    }

    private AspectContext createGraphicsContext(AtomicReference<Window> window) {
        AspectContext graphics_context = new AspectContext();
        graphics_context.onStart(() -> {
            window.set(new Window("title", 800, 600, false));
            System.out.println("aspect started");
            glClearColor(0.22f, 0.22f, 0.20f, 1);
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);
            synchronized (StrobeContext.this) {
                graphics_initialized = true;
                StrobeContext.this.notify();
            }
        });
        graphics_context.beforeRender(() -> {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        });
        graphics_context.afterRender(() -> {
            window.get().swapBuffers();
            window.get().pollEvents();
            if (window.get().shouldClose()) {
                Marshal.stop();
            }
        });
        graphics_context.onStop(() -> {
            System.out.println("aspect stoped");
            window.get().close();
            window.get().dispose();
        });
        return graphics_context;
    }

    private MarshalContext createGraphicsEcs(AspectContext graphics) {
        MarshalContext graphics_ecs_context = new MarshalContext(graphics.getThread());

        graphics.onStart(() -> {
            graphics_ecs_context.init();
            synchronized (StrobeContext.this) {
                graphics_ecs_initialized = true;
                StrobeContext.this.notify();
            }
        });
        graphics.beforeRender(() -> graphics_ecs_context.update());
        graphics.onStop(() -> graphics_ecs_context.dispose());

        Marshal.addContext(graphics_ecs_context);
        return graphics_ecs_context;
    }

    private DebugContext setupDebugContext(AspectContext graphics_context, AtomicReference<Window> window) {
        DebugContext debug_context = new DebugContext(window);
        debug_context.onInit(() -> {
            synchronized (StrobeContext.this) {
                debug_gui_initialized = true;
                StrobeContext.this.notify();
            }
        });
        graphics_context.addNode(debug_context.getDebugRoot(), AspectLocation.sroot().priority(Integer.MAX_VALUE));
        return debug_context;
    }

    public void addEntity(MarshalEntity entity) {
        ecs.add(entity);
    }

    public void removeEntity(MarshalEntity entity) {
        ecs.remove(entity);
    }

    public void addSystem(MarshalSystem system) {
        ecs.add(system);
    }

    public void removeSystem(MarshalSystem system) {
        ecs.remove(system);
    }

    public void addGraphicsSystem(MarshalSystem system) {
        graphics_ecs.add(system);
    }

    public void removeGraphicsSystem(MarshalSystem system) {
        graphics_ecs.remove(system);
    }

    public void addRenderNode(AspectNode graph_node, AspectLocation graph_location) {
        graphics.addNode(graph_node, graph_location);
    }

    public void removeRenderNode(AspectLocation graph_location) {
        graphics.removeNode(graph_location);
    }

    public void addDebugGui(String gui_name, AspectPass gui_pass) {
        debug_gui.addGui(gui_name, gui_pass);
    }

    public void removeDebugGui(String gui_name) {
        debug_gui.removeGui(gui_name);
    }

    public void removeDebugGui(AspectPass gui_pass) {
        debug_gui.removeGui(gui_pass.toString());
    }

    public List<MarshalEntity> getEntities(MarshalFamily.MarshalFamilyIdentifier family) {
        return ecs.getEntities(family);
    }


    public static int DEBUG = 0b0;

    public void addFlag(int flag) {
        if (flag == DEBUG) debug_flag = true;
    }

    public void removeFlag(int flag) {
        if (flag == DEBUG) debug_flag = false;
    }


}