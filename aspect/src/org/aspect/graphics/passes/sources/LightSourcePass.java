package org.aspect.graphics.passes.sources;

import org.aspect.Aspect;
import org.aspect.graphics.bindable.UniformBuffer;
import org.aspect.graphics.passes.Pass;
import org.joml.Vector3f;

public class LightSourcePass extends Pass {


    public static final String LIGHT_UBO = "lightUbo";

    private final UniformBuffer lightUbo;

    public LightSourcePass(String name, int bindingIndex) {
        super(name);

        lightUbo = Aspect.createUniformBuffer(bindingIndex,
                "vec3 position");

        lightUbo.uniformVec3("position", new Vector3f(50, 100, 150));
    }

    @Override
    public void setupSinksAndSources() {
        registerSource(UniformBuffer.class,LIGHT_UBO, lightUbo);
    }

    @Override
    public void complete() {
    }

    @Override
    public void execute() {
        lightUbo.uniformVec3("position",new Vector3f(50,100,150));
    }
}
