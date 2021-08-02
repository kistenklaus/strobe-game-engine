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

    private final Uniform<Vector3f> ambientLight;

    private final Uniform<Integer> directionalLightCount;
    private final Uniform<Vector3f>[] directionalLightDir;
    private final Uniform<Vector3f>[] directionalLightDiffuse;
    private final Uniform<Vector3f>[] directionalLightSpecular;

    private final int dirLightCount;
    private final int pointLightCount;

    public LightUbo(Graphics gfx, int dirLightCount, int pointLightCount) {
        super(gfx, "lights", 1,
                "vec3 ambientLight",
                "int directionalLightCount",
                "vec3["+dirLightCount+"] directionalLightDir",
                "vec3["+dirLightCount+"] directionalLightDiffuse",
                "vec3["+dirLightCount+"] directionalLightSpecular",
                "vec3[" + pointLightCount + "] pointLightPositions",
                "vec3[" + pointLightCount + "] pointLightAttenuations",
                "vec3[" + pointLightCount + "] pointLightDiffuses",
                "vec3[" + pointLightCount + "] pointLightSpeculars");
        this.dirLightCount = dirLightCount;
        this.pointLightCount = pointLightCount;
        ambientLight = getUniform(Vector3f.class, "ambientLight");

        directionalLightCount = getUniform(int.class, "directionalLightCount");

        directionalLightDir = getUniformArray(Vector3f.class, "directionalLightDir");
        directionalLightDiffuse = getUniformArray(Vector3f.class, "directionalLightDiffuse");
        directionalLightSpecular = getUniformArray(Vector3f.class, "directionalLightSpecular");
    }

    public void uniformAmbientLight(Graphics gfx, Vector3f ambientLight){
        if(ambientLight == null){
            this.ambientLight.set(gfx, new Vector3f(1));
        }else{
            this.ambientLight.set(gfx, ambientLight);
        }
    }

    public void uniformDirectionalLight(Graphics gfx, List<DirectionalLight> lights) {
        int n = Math.min(lights.size(), dirLightCount);
        directionalLightCount.set(gfx, n);
        for(int i=0;i<n;i++){
            directionalLightDir[i].set(gfx, lights.get(i).getDirection());
            directionalLightDiffuse[i].set(gfx, lights.get(i).getDiffuse());
            directionalLightSpecular[i].set(gfx, lights.get(i).getSpecular());
        }
    }
}
