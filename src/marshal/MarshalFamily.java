package marshal;

import java.util.HashSet;
import java.util.LinkedList;

public class MarshalFamily {

    private final MarshalComponentBits all, one;

    private LinkedList<MarshalEntity> members = new LinkedList<>();
    private HashSet<MarshalEntity> members_set = new HashSet<>();

    MarshalFamily(MarshalFamilyIdentifier family_identifier) {
        this.all = family_identifier.all;
        this.one = family_identifier.one;
    }

    @SafeVarargs
    public static MarshalFamilyIdentifier all(Class<? extends MarshalComponent>... component_classes) {
        return new MarshalFamilyIdentifier().all(component_classes);
    }

    @SafeVarargs
    public static MarshalFamilyIdentifier one(Class<? extends MarshalComponent>... component_classes) {
        return new MarshalFamilyIdentifier().one(component_classes);
    }

    public static MarshalFamilyIdentifier instanceOf(Class<? extends MarshalComponent> component_class) {
        return new MarshalFamilyIdentifier().instanceOf(component_class);
    }

    void takeIn(MarshalEntity entity) {
        if (!members_set.contains(entity)) {
            members.add(entity);
            members_set.add(entity);
        }
    }

    void exile(MarshalEntity entity) {
        if (members_set.contains(entity)) {
            members_set.remove(entity);
            members.remove(entity);
        }
    }

    LinkedList<MarshalEntity> getEntities() {
        return members;
    }

    boolean dependsOnComponent(MarshalComponentType component_type) {
        return all.has(component_type) || one.has(component_type);
    }

    boolean matches(MarshalEntity entity) {
        if (one.hasOne(entity.componentBits)) return true;
        if (all.containsAll(entity.componentBits)) return true;
        return false;
    }

    public static class MarshalFamilyIdentifier {
        private final MarshalComponentBits all = new MarshalComponentBits(), one = new MarshalComponentBits();
        private String str = null;

        public MarshalFamilyIdentifier() {
        }

        private static String bitsToString(MarshalComponentBits bits) {
            StringBuilder str_builder = new StringBuilder();
            str_builder.append("(");
            for (int i = 0; i < bits.length(); i++) {
                if (bits.has(i)) {
                    str_builder.append(i + ",");
                }
            }
            str_builder.deleteCharAt(str_builder.length() - 1);
            str_builder.append(")");
            return str_builder.toString();
        }

        @SafeVarargs
        public final MarshalFamilyIdentifier all(Class<? extends MarshalComponent>... component_classes) {
            one.clear();
            for (Class<? extends MarshalComponent> component_class : component_classes) {
                MarshalComponentType component_type = MarshalComponentType.get(component_class);
                all.put(component_type);
            }
            str = null;
            return this;
        }

        public final MarshalFamilyIdentifier instanceOf(Class<? extends MarshalComponent> component_class) {
            all.clear();
            MarshalComponentType component_type = MarshalComponentType.get(component_class);
            one.put(component_type);


            for (int i = 0; i < component_type.inherited_by.length(); i++) {
                if (component_type.inherited_by.has(i)) one.put(i);
            }

            str = null;
            return this;
        }

        @SafeVarargs
        public final MarshalFamilyIdentifier one(Class<? extends MarshalComponent>... component_classes) {
            all.clear();
            for (Class<? extends MarshalComponent> component_class : component_classes) {
                MarshalComponentType component_type = MarshalComponentType.get(component_class);
                one.put(component_type);
            }
            str = null;
            return this;
        }

        public String toString() {
            if (str == null) {
                str = "FamilyID:[all:" + bitsToString(all) + ",one:" + bitsToString(one) + "]";
            }
            return str;
        }


    }
}
