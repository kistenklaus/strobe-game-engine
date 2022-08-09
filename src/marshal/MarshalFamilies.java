package marshal;

import java.util.HashMap;
import java.util.LinkedList;

class MarshalFamilies {

    private final MarshalStaticArray<MarshalEntity> entities;

    private final MarshalStaticArray<MarshalFamily> families = new MarshalStaticArray<>();
    private final HashMap<String, Integer> family_indices = new HashMap<>();

    private final LinkedList<FamilyChange> change_queue = new LinkedList<>();

    MarshalFamilies(final MarshalStaticArray<MarshalEntity> entities) {
        this.entities = entities;
    }

    synchronized void applyFamilyChanges() {
        while (change_queue.size() > 0) {
            change_queue.removeFirst().apply();
        }
    }

    void enqueueEntityAdded(MarshalEntity entity) { change_queue.add(new EntityAdded(entity)); }

    void enqueueEntityRemoved(MarshalEntity entity) {
        change_queue.add(new EntityRemoved(entity));
    }

    void enqueueComponentChange(MarshalComponent component) {
        change_queue.add(new ComponentModified(component));
    }

    boolean haveChanged() {
        return change_queue.size() > 0;
    }

    synchronized MarshalFamily getFamily(MarshalFamily.MarshalFamilyIdentifier family_identifier) {
        Integer family_index = family_indices.get(family_identifier.toString());
        MarshalFamily family;
        if (family_index == null) {
            family = new MarshalFamily(family_identifier);
            family_index = families.add(family);
            family_indices.put(family_identifier.toString(), family_index);
            //check the family conditions against all entities;
            for (MarshalEntity entity : entities) {
                if (family.matches(entity)) {
                    family.takeIn(entity);
                }
            }
        } else {
            family = families.get(family_index);
        }
        return family;
    }

    private abstract class FamilyChange {
        abstract void apply();
    }


    private final class EntityAdded extends FamilyChange {
        private final MarshalEntity entity;

        public EntityAdded(MarshalEntity entity) {
            this.entity = entity;
        }

        @Override
        void apply() {
            for (MarshalFamily family : families) {
                if (family.matches(entity)) {
                    family.takeIn(entity);
                } else {
                    family.exile(entity);
                }
            }
        }
    }

    private final class EntityRemoved extends FamilyChange {
        private final MarshalEntity entity;

        public EntityRemoved(MarshalEntity entity) {
            this.entity = entity;
        }

        void apply() {
            for (MarshalFamily family : families) {
                family.exile(entity);
            }
        }
    }

    private final class ComponentModified extends FamilyChange {
        private final MarshalComponent component;

        public ComponentModified(MarshalComponent component) {
            this.component = component;
        }

        @Override
        void apply() {
            for (MarshalFamily family : families) {
                if (family.dependsOnComponent(MarshalComponentType.get(component.getClass()))) {
                    if (family.matches(component.root)) {
                        family.takeIn(component.root);
                    } else {
                        family.exile(component.root);
                    }
                }
            }
        }

    }
}
