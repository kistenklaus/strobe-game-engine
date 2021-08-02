package org.strobe.gfx.rendergraph.common.manager;

import org.strobe.gfx.Graphics;
import org.strobe.gfx.lights.DirectionalLight;
import org.strobe.gfx.lights.LightUbo;

import java.awt.*;
import java.util.List;
import java.util.ArrayList;

public final class LightManager {

    private static final int POINT_LIGHT_COUNT = 10;
    private static final int DIR_LIGHT_COUNT = 3;

    private ArrayList<DirectionalLight> directionalLights = new ArrayList<>();
    private final LightUbo ubo;

    public LightManager(Graphics gfx){
        ubo = new LightUbo(gfx, DIR_LIGHT_COUNT, POINT_LIGHT_COUNT);
    }

    public void submitLight(DirectionalLight light){
        directionalLights.add(light);
    }

    public void clearFrame(){
        directionalLights.clear();
    }

    public LightUbo ubo(){
        return ubo;
    }

}
