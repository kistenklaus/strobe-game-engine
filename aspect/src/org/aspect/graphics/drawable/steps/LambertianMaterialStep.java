package org.aspect.graphics.drawable.steps;

import org.aspect.Aspect;
import org.aspect.graphics.data.Material;

public class LambertianMaterialStep extends MaterialStep {

    private static final String VERTEX_PATH = "assets/shaders/lambertian.vs";
    private static final String FRAGMENT_PATH = "assets/shaders/lambertian.fs";

    public LambertianMaterialStep(String passName, Material material) {
        super(passName, Aspect.loadShader(VERTEX_PATH, FRAGMENT_PATH), material);
    }
}
