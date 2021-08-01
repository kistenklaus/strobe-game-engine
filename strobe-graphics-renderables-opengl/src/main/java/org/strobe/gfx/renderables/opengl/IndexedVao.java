package org.strobe.gfx.renderables.opengl;

import org.strobe.gfx.Graphics;
import org.strobe.gfx.Renderable;
import org.strobe.gfx.opengl.bindables.vao.Vao;

import static org.lwjgl.opengl.GL11.glDrawElements;

public class IndexedVao implements Renderable {

    private final Vao vao;
    private int drawCount;
    private final int type;
    private final int mode;

    public IndexedVao(Vao vao, int drawCount, int type,int mode){
        this.vao = vao;
        this.drawCount = drawCount;
        this.type = type;
        this.mode = mode;
    }

    @Override
    public void render(Graphics gfx) {
        gfx.bind(vao);
        glDrawElements(mode, drawCount, type, 0);
        gfx.unbind(vao);
    }
}
