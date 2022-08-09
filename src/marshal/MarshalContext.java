package marshal;

import java.util.ArrayList;
import java.util.LinkedList;

public final class MarshalContext implements Runnable {

    private boolean running = false;
    private boolean updating = false;
    private boolean waiting = false;

    private final ArrayList<MarshalSystem> systems = new ArrayList<>();

    private LinkedList<MarshalCallback> scheduled_entity_changes = new LinkedList<>();

    private final MarshalSignal<MarshalEntity> entity_added = new MarshalSignal<>();
    private final MarshalSignal<MarshalEntity> entity_removed = new MarshalSignal<>();

    private final MarshalSignal<MarshalComponent> component_added = new MarshalSignal<>();
    private final MarshalSignal<MarshalComponent> component_removed = new MarshalSignal<>();

    private MarshalCallback context_init = null;
    private MarshalCallback context_dispose = null;
    private MarshalCallback context_pre_cycle = null;
    private MarshalCallback context_post_cycle = null;

    private final LinkedList<MarshalEvent> event_queue = new LinkedList<>();

    private final LinkedList<MarshalCallback> scheduled = new LinkedList<>();

    private long last_cycle;

    private Thread contextThread = null;
    final boolean threaded;

    public MarshalContext() {
        this(true);
        this.contextThread = new Thread(this, "marshal_thread");
    }

    public MarshalContext(Thread contextThread) {
        this(false);
        this.contextThread = contextThread;
    }

    private MarshalContext(boolean threaded) {
        this.threaded = threaded;
        component_added.add(component -> {
            int component_index = MarshalComponentType.getIndex(component.getClass());
            MarshalSignal<MarshalComponent> signal = classified_component_added.get(component_index);
            if (signal != null) {
                signal.dispatch(component);
            }
        });
        component_removed.add(component -> {
            int component_index = MarshalComponentType.getIndex(component.getClass());
            MarshalSignal<MarshalComponent> signal = classified_component_removed.get(component_index);
            if (signal != null) {
                signal.dispatch(component);
            }
        });
        entity_added.add(entity -> {
            for (MarshalComponent component : entity.components) {
                component_added.dispatch(component);
            }
        });
        entity_removed.add(entity -> {
            for (MarshalComponent component : entity.components) {
                component_removed.dispatch(component);
            }
        });
    }


    public final void start() {
        contextThread.start();
    }

    public final void stop() {
        running = false;
    }

    public final void init() {
        if (context_init != null) context_init.callback();
        for (MarshalSystem system : systems) system.init();

        running = true;

        waiting = true;
        Marshal.waitForFamilySync(this);
        waiting = false;

        last_cycle = System.nanoTime();
        updating = true;
    }

    public final void update() {
        long current_cycle = System.nanoTime();
        float delta_time = (current_cycle - last_cycle) * 1e-9f;
        last_cycle = current_cycle;

        if (context_pre_cycle != null) context_pre_cycle.callback();
        for (MarshalSystem system : systems) {
            system.update(delta_time);
        }
        if (context_pre_cycle != null) context_post_cycle.callback();
        /*
         * Event Processing:
         */
        synchronized (this) {
            while (event_queue.size() > 0) {
                MarshalEvent event = event_queue.removeFirst();
                event.poll(this);
            }
        }
        /*
         * Scheduled stuff professing
         */
        synchronized (this) {
            while (scheduled.size() > 0) {
                MarshalCallback callback = scheduled.removeFirst();
                callback.callback();
            }
        }
        /*
         * Family Synchronization:
         *
         * -if any family has changed => wait for all contexts to wait
         * after that open a new Thread to update all changed families
         * when all families are updated resume all Contexts:
         */
        if (Marshal.getFamilies().haveChanged() || Marshal.entitiesHaveChanged()) {
            waiting = true;
            Marshal.waitForFamilySync(this);
            waiting = false;
        }
    }

    public final void dispose() {
        updating = false;
        if (context_dispose != null) context_dispose.callback();
        for (MarshalSystem system : systems) system.cleanUp();
    }

    @Override
    public final void run() {
        init();
        while (running) {
            update();
        }
        dispose();
    }

    public final synchronized void add(MarshalSystem system) {
        if (!updating) {
            system.init();
            systems.add(system);
        } else {
            scheduled.add(() -> {
                system.init();
                systems.add(system);
            });
        }
    }

    public final synchronized void remove(MarshalSystem system) {
        systems.remove(system);
    }

    public final void add(MarshalEntity entity) {
        Marshal.scheduleEntityAdd(entity);
    }

    public final void remove(MarshalEntity entity) {
        Marshal.scheduledEntityRemove(entity);
    }

    public final LinkedList<MarshalEntity> getEntities(MarshalFamily.MarshalFamilyIdentifier family_identifier) {
        return Marshal.getEntities(family_identifier);
    }

    public final void on(int event_type, MarshalListener<MarshalEntity> entity_listener) {

        switch (event_type) {
            case Marshal.ENTITY_ADDED:
                entity_added.add(entity_listener);
                break;
            case Marshal.ENTITY_REMOVED:
                entity_removed.add(entity_listener);
                break;
            default:
                throw new IllegalArgumentException("COMPONENT_ADDED or COMPONENT_REMOVED requires arguments on(type, component_class, listener)");
        }
    }

    MarshalStaticArray<MarshalSignal<MarshalComponent>> classified_component_added = new MarshalStaticArray<>();
    MarshalStaticArray<MarshalSignal<MarshalComponent>> classified_component_removed = new MarshalStaticArray<>();


    public final void on(int event_type, Class<? extends MarshalComponent> component_class, MarshalListener<MarshalComponent> component_listener) {
        int component_index;
        switch (event_type) {
            case Marshal.COMPONENT_ADDED:
                component_index = MarshalComponentType.getIndex(component_class);
                classified_component_added.computeIfNull(component_index, () -> new MarshalSignal<>()).add(component_listener);
                break;
            case Marshal.COMPONENT_REMOVED:
                component_index = MarshalComponentType.getIndex(component_class);
                classified_component_removed.computeIfNull(component_index, () -> new MarshalSignal<>()).add(component_listener);
                ;
                break;
            default:
                throw new IllegalArgumentException("ENTITY_ADDED or ENTITY_REMOVED requires arguments on(type, listener)");
        }
    }

    public final void on(int event_type, MarshalCallback callback) {
        switch (event_type) {
            case Marshal.CONTEXT_INIT:
                this.context_init = callback;
                break;
            case Marshal.CONTEXT_DISPOSED:
                this.context_dispose = callback;
                break;
            case Marshal.CONTEXT_PRE_UPDATE:
                this.context_pre_cycle = callback;
                break;
            case Marshal.CONTEXT_POST_UPDATE:
                this.context_post_cycle = callback;
                break;
            default:
                throw new MarshalException("overloaded function only takes context events");
        }
    }

    final synchronized void enqueueEvent(MarshalEvent event) {
        event_queue.add(event);
    }

    final void dispatchEntityAdded(MarshalEntity entity) {
        entity_added.dispatch(entity);
    }

    final void dispatchEntityRemoved(MarshalEntity entity) {
        entity_removed.dispatch(entity);
    }

    final void dispatchComponentAdded(MarshalComponent component) {
        component_added.dispatch(component);
    }

    final void dispatchComponentRemoved(MarshalComponent component) {
        component_removed.dispatch(component);
    }

    final boolean isWaiting() {
        return waiting;
    }

    final boolean isUpdating() {
        return updating;
    }

    public interface MarshalCallback {
        void callback();
    }
}
