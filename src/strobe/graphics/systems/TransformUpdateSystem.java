package strobe.graphics.systems;

import aspect.Aspect;
import marshal.MarshalComponentMapper;
import marshal.MarshalEntity;
import marshal.MarshalFamily;
import marshal.MarshalSystem;
import org.joml.Matrix4f;
import org.joml.Vector3f;
import strobe.Strobe;
import strobe.components.Transform;

import java.util.List;

public class TransformUpdateSystem extends MarshalSystem {

    private List<MarshalEntity> transform_entities;
    private MarshalComponentMapper<Transform> transform_mapper = MarshalComponentMapper.getFor(Transform.class);

    @Override
    public void init() {
        if (!Thread.currentThread().getName().equals(Aspect.getThreadName())) {
            System.err.println("The TransformUpdateSystem is supposed to be used on a ecs connected to aspect");
        }
        transform_entities = Strobe.getEntities(MarshalFamily.all(Transform.class));
    }

    @Override
    public void update(float dt) {
        for (MarshalEntity entity : transform_entities) {
            Transform transform = transform_mapper.get(entity);
            Matrix4f transformation = new Matrix4f();

            transformation.translate(transform.getPosition());

            Vector3f rc = transform.getRotationCenter();
            transformation.translate(rc);
            transformation.rotate((float)Math.toRadians(transform.getRotation().x), 1.0f, 0, 0);
            transformation.rotate((float)Math.toRadians(transform.getRotation().y), 0, 1.0f, 0);
            transformation.rotate((float)Math.toRadians(transform.getRotation().z), 0, 0, 1.0f);
            transformation.translate(-rc.x, -rc.y, rc.z);

            transformation.scale(transform.getScale());

            transform.setTransformationMatrix(transformation);
        }
    }

    @Override
    public void cleanUp() {

    }
}
