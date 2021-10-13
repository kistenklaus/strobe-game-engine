package org.strobe.ecs.context.renderer.materials;

import org.joml.Vector3f;
import org.strobe.ecs.context.renderer.EntityRenderer;
import org.strobe.gfx.Graphics;
import org.strobe.gfx.materials.LambertianColorShaderMaterial;
import org.strobe.gfx.materials.LambertianTextureShaderMaterial;
import org.strobe.gfx.materials.ShaderMaterial;
import org.strobe.gfx.opengl.bindables.texture.Texture2D;

public final class LambertianMaterial extends Material{

    private LambertianColorShaderMaterial colorShader;
    private LambertianTextureShaderMaterial textureShader;

    private final Vector3f diffuseColor = new Vector3f();
    private Texture2D diffuseTexture = null;

    private boolean textureShaderSelected = false;

    public LambertianMaterial(Vector3f diffuseColor){
        super();
        setDiffuseColor(diffuseColor);
    }

    public LambertianMaterial(float r, float g, float b){
        this(new Vector3f(r,g,b));
    }

    public LambertianMaterial(Vector3f diffuseColor, Texture2D diffuseTexture){
        setDiffuseColor(diffuseColor);
        setDiffuseTexture(diffuseTexture);
    }

    @Override
    protected ShaderMaterial createShaderMaterials(Graphics gfx, EntityRenderer renderer) {
        colorShader = new LambertianColorShaderMaterial(gfx);
        textureShader = new LambertianTextureShaderMaterial(gfx);

        textureShaderSelected = diffuseTexture!=null;
        return textureShaderSelected?textureShader:colorShader;
    }

    public void setDiffuseColor(final Vector3f diffuseColor){
        materialOps.add((gfx,r)-> {
            this.diffuseColor.set(diffuseColor);
            if(textureShaderSelected){
                textureShader.setDiffuseColor(diffuseColor);
            }else{
                colorShader.setDiffuseColor(diffuseColor);
            }
        });
    }

    public void setDiffuseTexture(final Texture2D diffuseTexture){
        materialOps.add((gfx, r)->{
            this.diffuseTexture = diffuseTexture;
            if(!textureShaderSelected && this.diffuseTexture!=null){
                switchShaderMaterial(textureShader);
                textureShader.setDiffuseColor(diffuseColor);
                textureShader.setDiffuseTexture(diffuseTexture);
            }
            else if(textureShaderSelected && this.diffuseTexture==null){
                switchShaderMaterial(colorShader);
                colorShader.setDiffuseColor(diffuseColor);
            }
        });
    }

    public Vector3f getDiffuseColor(){
        return colorShader.getDiffuseColor();
    }
}
