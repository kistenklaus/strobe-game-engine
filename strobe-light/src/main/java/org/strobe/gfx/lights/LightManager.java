package org.strobe.gfx.lights;

import org.strobe.gfx.Graphics;
import org.strobe.gfx.lights.DirectionalLight;

import java.awt.*;
import java.util.List;
import java.util.ArrayList;

public final class LightManager {

    private ArrayList<DirectionalLight> directionalLights = new ArrayList<>();
    private final LightUbo ubo;

    public LightManager(Graphics gfx, int dirLightCount, int pointLightCount){
        ubo = new LightUbo(gfx, dirLightCount, pointLightCount);
    }


    public void addLight(DirectionalLight light){
        directionalLights.add(light);
    }

    public void removeLight(DirectionalLight light){
        directionalLights.remove(light);
    }

    public List<DirectionalLight> directionalLights(){
        return directionalLights;
    }

    public LightUbo ubo(){
        return ubo;
    }

}
