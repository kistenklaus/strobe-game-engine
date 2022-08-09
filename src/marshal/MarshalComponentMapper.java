package marshal;

/**
 * @version 1.2
 * @author Karl
 */
public class MarshalComponentMapper<T extends MarshalComponent>{

    private final MarshalComponentType component_type;
    private MarshalComponentMapper(MarshalComponentType type){
        this.component_type = type;
    }

    public static <T extends MarshalComponent> MarshalComponentMapper getFor(Class<T> component_class){
        MarshalComponentType type = MarshalComponentType.get(component_class);
        return new MarshalComponentMapper<T>(type);
    }

    public T get(MarshalEntity entity){
        T res = (T)entity.query(component_type.index());
        if(res != null)return res;
        for(int i=0;i<component_type.inherited_by.length();i++){
            if(component_type.inherited_by.has(i) && entity.componentBits.has(i)){
                res = (T)entity.query(i);
                if(res != null)return res;
            }
        }
        return null;
    }
}
