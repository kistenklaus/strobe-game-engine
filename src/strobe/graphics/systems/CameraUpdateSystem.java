package strobe.graphics.systems;

import marshal.MarshalComponentMapper;
import marshal.MarshalEntity;
import marshal.MarshalFamily;
import marshal.MarshalSystem;
import strobe.Strobe;
import strobe.components.Camera;
import strobe.components.OrthographicCamera;
import strobe.components.Transform;
import strobe.graphics.bindables.UBO;

import java.util.List;

public class CameraUpdateSystem extends MarshalSystem {

    private final MarshalComponentMapper<Camera> camera_mapper = MarshalComponentMapper.getFor(Camera.class);
    private final MarshalComponentMapper<Transform> transform_mapper = MarshalComponentMapper.getFor(Transform.class);
    private List<MarshalEntity> camera_entities;

    @Override
    public void init() {
        camera_entities = Strobe.getEntities(MarshalFamily.instanceOf(Camera.class));

    }

    @Override
    public void update(float dt) {
        System.out.println(camera_entities);
        for (MarshalEntity entity : camera_entities) {
            Camera camera = camera_mapper.get(entity);
            if(camera==null)return;
            //System.out.println("cameraEntity : " + entity);
            UBO camera_ubo = camera.getCameraUbo();
            //updating the proj matrix
            camera.updateProjection();
            //camera_ubo.uniformMat4(Camera.PROJ_MATRIX_NAME, camera.getProjection());

            //updating the view matrix
            Transform transform = transform_mapper.get(entity);
            if(transform==null)return;
            camera_ubo.uniformMat4(Camera.VIEW_MATRIX_NAME,
                    transform.getTransformationMatrix().invert());
        }
    }

    @Override
    public void cleanUp() {

    }
}
