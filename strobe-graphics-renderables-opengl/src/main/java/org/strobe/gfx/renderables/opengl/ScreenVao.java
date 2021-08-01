package org.strobe.gfx.renderables.opengl;

import org.strobe.gfx.Graphics;
import org.strobe.gfx.Renderable;
import org.strobe.gfx.opengl.bindables.vao.Ibo;
import org.strobe.gfx.opengl.bindables.vao.Vao;

import static org.lwjgl.opengl.GL11.*;

public final class ScreenVao implements Renderable {

    private static Vao screenVao;

    public ScreenVao(Graphics gfx){
        if(screenVao != null){
            screenVao = new Vao(gfx, 4, new Ibo(gfx, new int[]{3,2,1,0,2,3}, true),
                    "layout(location=0,usage=static) in vec3",
                            "layout(location=1,usage=static) in vec2");
            screenVao.bufferLocation(gfx, 0, new float[]{
                    -1,-1,0,
                    -1,1,0,
                    1,1,0,
                    1,-1,0
            });
            screenVao.bufferLocation(gfx, 1, new float[]{
                    0,0,
                    0,1,
                    1,1,
                    1,0
            });
        }
    }

    @Override
    public void render(Graphics gfx) {
        gfx.bind(screenVao);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        gfx.unbind(screenVao);
    }
}
