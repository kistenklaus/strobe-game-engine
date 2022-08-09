package strobe.components;

import aspect.Aspect;
import aspect.AspectLocation;
import aspect.AspectNode;
import marshal.MarshalComponent;
import org.joml.Matrix4f;
import strobe.Strobe;
import strobe.graphics.bindables.UBO;

public abstract class Camera extends MarshalComponent {

    public static final int CAMERA_BINDING_INDEX = 0;
    public static final String VIEW_MATRIX_NAME = "view";
    public static final String PROJ_MATRIX_NAME = "proj";

    private final UBO camera_ubo;
    private boolean projection_changed = false;
    private final Matrix4f projection;

    public Camera(AspectNode target){
        camera_ubo = new UBO("camera", CAMERA_BINDING_INDEX, "mat4 "+VIEW_MATRIX_NAME);
        AspectNode camera_node = Aspect.createNode("camera-node-[@."+hashCode()+"]");
        camera_node.appendChild(target);
        camera_node.addBindable(camera_ubo);
        Strobe.addRenderNode(camera_node, AspectLocation.sroot());

        this.projection = new Matrix4f().identity();
    }

    public synchronized void updateProjection(){
        if(projection_changed){
            projection.set(calculateProjection());
            projection_changed = false;
        }
    }

    protected synchronized void forceProjectionUpdate(){
        projection.set(calculateProjection());
        projection_changed = false;
    }

    public abstract Matrix4f calculateProjection();

    public synchronized Matrix4f getProjection(){
        return new Matrix4f(projection);
    };

    public UBO getCameraUbo(){
        return camera_ubo;
    }





}
