package org.strobe.gfx.lights;

import org.joml.Vector3f;
import org.strobe.gfx.Graphics;
import org.strobe.gfx.Pool;
import org.strobe.gfx.opengl.bindables.mapper.Uniform;
import org.strobe.gfx.opengl.bindables.ubo.Ubo;
import org.strobe.gfx.opengl.bindables.ubo.UboPool;

import java.util.List;


@Pool(UboPool.class)
public final class LightUbo extends Ubo {

    private final Uniform<Integer> directionalLightCount;
    private final Uniform<Vector3f>[] directionalLightDir;
    private final Uniform<Vector3f>[] directionalLightAmbient;
    private final Uniform<Vector3f>[] directionalLightDiffuse;
    private final Uniform<Vector3f>[] directionalLightSpecular;

    public LightUbo(Graphics gfx) {
        super(gfx, "lights", 1,
                "int directionalLightCount",
                "vec3["+ LightConstants.DIRECTIONAL_LIGHT_COUNT+"] directionalLightDir",
                "vec3["+ LightConstants.DIRECTIONAL_LIGHT_COUNT+"] directionalLightAmbient",
                "vec3["+ LightConstants.DIRECTIONAL_LIGHT_COUNT+"] directionalLightDiffuse",
                "vec3["+ LightConstants.DIRECTIONAL_LIGHT_COUNT+"] directionalLightSpecular");

        directionalLightCount = getUniform(int.class, "directionalLightCount");
        directionalLightDir = getUniformArray(Vector3f.class, "directionalLightDir");
        directionalLightAmbient = getUniformArray(Vector3f.class, "directionalLightAmbient");
        directionalLightDiffuse = getUniformArray(Vector3f.class, "directionalLightDiffuse");
        directionalLightSpecular = getUniformArray(Vector3f.class, "directionalLightSpecular");
    }

    public void uniformDirectionalLight(Graphics gfx, List<DirectionalLight> lights) {
        directionalLightCount.set(gfx, lights.size());
        for(int i=0;i<lights.size();i++){
            directionalLightDir[i].set(gfx, lights.get(i).getDirection());
            directionalLightAmbient[i].set(gfx, lights.get(i).getAmbient());
            directionalLightDiffuse[i].set(gfx, lights.get(i).getDiffuse());
            directionalLightSpecular[i].set(gfx, lights.get(i).getSpecular());
        }
    }
}
