package org.strobe.game;

import org.joml.Matrix4f;
import org.joml.Quaternionf;
import org.joml.Vector2f;
import org.joml.Vector3f;
import org.strobe.ecs.*;
import org.strobe.ecs.context.renderer.transform.Transform;

public final class LookAtCenterSystem extends EntitySystem {

    private EntityPool pool;

    public LookAtCenterSystem(EntityComponentSystem ecs) {
        super(ecs);
        pool = ecs.createPool(EntityFilter.requireAll(Transform.class, LookAtCenter.class));
    }

    @Override
    public void init() {

    }

    @Override
    public void update(float dt) {
        for (Entity entity : pool) {
            Transform transform = entity.get(Transform.class);
            Vector3f pos = transform.getTransformationMatrix().transformPosition(transform.getPosition(),
                    new Vector3f());
            Vector3f dir = pos.mul(-1);
            dir.normalize();


            Quaternionf orientation = transform.getOrientation();
            System.out.println("before : " + transform.getOrientation().normalize());
            System.out.println(transform.getOrientation().transform(new Vector3f(0,0,-1)));
            System.out.println(transform.getForward() +"  " + dir);
            Quaternionf lookAt = new Quaternionf().rotateTo(transform.getForward(), dir);

            System.out.println("lookAt " + lookAt);
            orientation.premul(lookAt);

            System.out.println(orientation);
            Vector3f newUp = orientation.transform(new Vector3f(0,1,0));


            Vector3f right = new Vector3f(0,-1,0).cross(dir);
            if(right.lengthSquared() > 0.01f){
                right.normalize();
                Vector3f up = right.cross(dir).normalize();
                Quaternionf adj = new Quaternionf().rotateTo(newUp, up);
                orientation.premul(adj);
                System.out.println(dir   + " " + right + "  " + up +"  " + newUp +"  "  +adj);
            }
            transform.getOrientation().normalize();

        }
    }


    @Override
    public void dispose() {

    }
}
