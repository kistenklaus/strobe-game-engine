package strobe.graphics.systems;

import marshal.MarshalComponentMapper;
import marshal.MarshalEntity;
import marshal.MarshalFamily;
import marshal.MarshalSystem;
import strobe.Strobe;
import strobe.components.Mesh;

import java.util.List;

public class MeshUpdateSystem extends MarshalSystem {
    private List<MarshalEntity> mesh_entities;
    private final MarshalComponentMapper<Mesh> mesh_mapper = MarshalComponentMapper.getFor(Mesh.class);
    @Override
    public void init() {
        mesh_entities = Strobe.getEntities(MarshalFamily.all(Mesh.class));
    }

    @Override
    public void update(float dt) {
        for(MarshalEntity entity : mesh_entities){
            Mesh mesh = mesh_mapper.get(entity);
        }
    }

    @Override
    public void cleanUp() {

    }
}
