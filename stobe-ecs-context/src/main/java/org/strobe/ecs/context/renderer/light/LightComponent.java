package org.strobe.ecs.context.renderer.light;

import org.joml.Vector3f;
import org.strobe.ecs.Component;
import org.strobe.ecs.context.renderer.EntityRenderer;
import org.strobe.gfx.Graphics;
import org.strobe.gfx.lights.AbstractLight;

import java.util.ArrayList;
import java.util.function.BiConsumer;

abstract class LightComponent implements Component {

    protected final AbstractLight light;

    private final ArrayList<BiConsumer<Graphics, EntityRenderer>> lightOps = new ArrayList<>();


    LightComponent(AbstractLight light){
        if(light == null)throw new IllegalArgumentException();
        this.light = light;
    }

    void enqueueLightOps(EntityRenderer renderer){
        renderer.enqueueRenderOps(lightOps);
        lightOps.clear();
    }


    public void setAmbientColor(Vector3f ambientColor){
        lightOps.add((gfx,r)->light.setAmbient(ambientColor));
    }

    public void setDiffuseColor(Vector3f diffuseColor){
        lightOps.add((gfx,r)->light.setDiffuse(diffuseColor));
    }

    public void setSpecularColor(Vector3f specularColor){
        lightOps.add((gfx,r)->light.setSpecular(specularColor));
    }

    AbstractLight getAbstractLight(){
        return light;
    }
}
