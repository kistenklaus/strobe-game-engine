package org.strobe.gfx.rendergraph.common.manager;

import org.joml.Vector3f;
import org.strobe.gfx.Graphics;
import org.strobe.gfx.lights.AbstractLight;
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

    public void submitLight(AbstractLight light){
        if(light.getClass() == DirectionalLight.class){
            directionalLights.add((DirectionalLight) light);
            return;
        }
        throw new UnsupportedOperationException();
    }

    public void updateUbo(Graphics gfx){
        ubo.uniformAmbientLight(gfx, calculateAmbient());
        ubo.uniformDirectionalLight(gfx, directionalLights);
    }

    private Vector3f calculateAmbient(){
        Vector3f acc = new Vector3f();
        for(DirectionalLight dirLight : directionalLights){
            acc.add(dirLight.getAmbient());
        }
        acc.mul(1f/Math.min(directionalLights.size(), DIR_LIGHT_COUNT));
        return acc;
    }

    public Iterable<DirectionalLight> directionalLights(){
        return directionalLights;
    }

    public void clearFrame(){
        directionalLights.clear();
    }

    public LightUbo ubo(){
        return ubo;
    }

}
