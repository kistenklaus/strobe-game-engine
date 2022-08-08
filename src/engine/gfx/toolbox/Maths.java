package engine.gfx.toolbox;

import org.joml.Matrix4f;
import org.joml.Vector3f;

public final class Maths {
	
	public static Matrix4f createTransformationMatrix(Vector3f translation, float rx, float ry, float rz, float scale) {
		Matrix4f matrix = new Matrix4f();
		matrix.identity();
		matrix.translate(translation);
		matrix.rotate((float)Math.toRadians(rx), new Vector3f(1,0,0));
		matrix.rotate((float)Math.toRadians(ry), new Vector3f(0,1,0));
		matrix.rotate((float)Math.toRadians(rz), new Vector3f(0,0,1));
		matrix.scale(scale);
		return matrix;
	}
	
	public static Matrix4f createViewMatrix(Vector3f camPos, float pitch, float yaw, float roll) {
		Matrix4f matrix = new Matrix4f();
		matrix.identity();
		matrix.rotate((float)Math.toRadians(pitch), new Vector3f(1,0,0), matrix);
		matrix.rotate((float)Math.toRadians(yaw), new Vector3f(0,1,0), matrix);
		matrix.rotate((float)Math.toRadians(roll), new Vector3f(0,0,1), matrix);
		matrix.translate(new Vector3f(camPos).mul(-1));
		return matrix;
	}
	
	
	public static Matrix4f createProjectionMatrix(int winWidth, int winHeight, float FOV, float FAR_PLANE, float NEAR_PLANE) {
		float aspectRatio = (float) winWidth / winHeight;
		float y_scale = (float) ((1f / Math.tan(Math.toRadians(FOV /2f))) * aspectRatio);
		float x_scale =  y_scale / aspectRatio;
		float frustum_length = FAR_PLANE -NEAR_PLANE;
		Matrix4f projectionMatrix = new Matrix4f();
		projectionMatrix.m00(x_scale);
		projectionMatrix.m11(y_scale);
		projectionMatrix.m22( -( (FAR_PLANE + NEAR_PLANE) / frustum_length) );
		projectionMatrix.m23(-1);
		projectionMatrix.m32( -( (2* NEAR_PLANE*FAR_PLANE) / frustum_length ) );
		projectionMatrix.m33(0);
		return projectionMatrix;
	}
	
}
