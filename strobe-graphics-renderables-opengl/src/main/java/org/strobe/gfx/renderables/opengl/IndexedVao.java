package org.strobe.gfx.renderables.opengl;

import org.strobe.gfx.Graphics;
import org.strobe.gfx.Renderable;
import org.strobe.gfx.opengl.bindables.vao.Vao;

import static org.lwjgl.opengl.GL11.*;

public class IndexedVao implements Renderable {

    private final Vao vao;
    private int drawCount;

    public IndexedVao(Vao vao, int drawCount){
        this.vao = vao;
        this.drawCount = drawCount;
    }

    @Override
    public void render(Graphics gfx) {
        gfx.bind(vao);
        glDrawElements(GL_TRIANGLES, drawCount, GL_UNSIGNED_INT, 0);
        gfx.unbind(vao);
    }
}
