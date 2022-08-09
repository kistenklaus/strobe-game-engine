package strobe.components;

import aspect.AspectNode;
import org.joml.Matrix4f;

public class OrthographicCamera extends Camera{

    public OrthographicCamera(AspectNode target){
        super(target);
    }

    @Override
    public Matrix4f calculateProjection() {
        return new Matrix4f().identity();
    }
}
