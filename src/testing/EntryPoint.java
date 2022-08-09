package testing;


import aspect.Aspect;
import aspect.AspectLocation;
import aspect.AspectNode;
import marshal.Marshal;
import marshal.MarshalEntity;
import marshal.MarshalFamily;
import marshal.MarshalSystem;
import org.joml.Vector3f;
import strobe.Strobe;
import strobe.components.Mesh;
import strobe.components.OrthographicCamera;
import strobe.components.Transform;
import strobe.debug.ComponentDebugGuiSystem;
import strobe.graphics.bindables.Shader;
import strobe.graphics.systems.CameraUpdateSystem;
import strobe.graphics.systems.TransformUpdateSystem;

import static org.lwjgl.opengl.GL11.GL_TRIANGLES;
import static org.lwjgl.opengl.GL11.glDrawElements;
import static org.lwjgl.opengl.GL11C.GL_UNSIGNED_INT;
import static org.lwjgl.opengl.GL15.GL_STATIC_DRAW;

public class EntryPoint {

    public static void main(String[] args) throws InterruptedException {
        Strobe.createContext();

        MarshalEntity entity = Marshal.createEntity();
        entity.add(new Transform(
                new Vector3f(0, 0, 0),
                new Vector3f(1, 1, 1),
                new Vector3f(0, 0, 0),
                new Vector3f(0, 0, 0)));
        Mesh mesh = new Mesh();
        mesh.setAttribute(0, 12, 3, 3, 0, 1, new float[]{
                -0.5f, 0.5f, 0f,
                0.5f, 0.5f, 0f,
                0.5f, -0.5f, 0f,
                -0.5f, -0.5f, 0f
        }, GL_STATIC_DRAW);
        mesh.setIndices(6, new int[]{
                0, 1, 2,
                2, 3, 0
        }, GL_STATIC_DRAW);

        entity.add(mesh);
        Strobe.addEntity(entity);

        Strobe.addGraphicsSystem(new TransformUpdateSystem());


        AspectNode scene = Aspect.createNode("scene-node");

        MarshalEntity camera = Marshal.createEntity();
        camera.add(new Transform());
        camera.add(new OrthographicCamera(scene));
        Strobe.addEntity(camera);

        Strobe.addGraphicsSystem(new CameraUpdateSystem());

        Strobe.addSystem(new MarshalSystem() {

            @Override
            public void init() {
                Shader entity_shader = new Shader("assets/shaders/helloWorld");
                for (MarshalEntity entity : Strobe.getEntities(MarshalFamily.all(Transform.class, Mesh.class))) {
                    Mesh mesh_component = entity.query(Mesh.class);
                    Transform transform_component = entity.query(Transform.class);
                    AspectNode entity_node = Aspect.createNode("entity-node-[" + entity.toString() + "]");

                    entity_node.addBindable(entity_shader);

                    entity_node.addBindable(mesh.createVao());
                    entity_node.setUpPass(() -> {
                        entity_shader.uniformMat4("transform", transform_component.getTransformationMatrix());
                        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
                    });

                    Strobe.addRenderNode(entity_node, AspectLocation.sis(scene));
                }
            }

            @Override
            public void update(float dt) {

            }

            @Override
            public void cleanUp() {

            }

        });

        Strobe.addSystem(new ComponentDebugGuiSystem());


    }
}
