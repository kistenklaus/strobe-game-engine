package org.strobe.gfx;

import org.strobe.debug.Debug;
import org.strobe.debug.Debuggable;

import java.util.ArrayList;

public final class MasterRenderer implements Debuggable {

    @Debug
    private ArrayList<Renderer> renderers = new ArrayList<>();
    private ArrayList<Integer> prios = new ArrayList<>();

    public void add(int renderPrio, Renderer renderer) {
        int insertionIndex = prios.size();
        for (int i = 0; i < prios.size(); i++) {
            if (renderPrio > prios.get(i)) {
                insertionIndex = i;
                break;
            }
        }
        prios.add(insertionIndex, renderPrio);
        renderers.add(insertionIndex, renderer);
    }

    public boolean remove(Renderer renderer){
        for(int i=0;i<renderers.size();i++){
            if(renderer.equals(renderers.get(i))){
                prios.remove(i);
                renderers.remove(i);
                return true;
            }
        }
        return false;
    }

    public void render(Graphics gfx){
        for(Renderer renderer : renderers)renderer.render(gfx);
    }

    public void dispose(Graphics gfx) {
        for(Renderer renderer : renderers){
            renderer.dispose(gfx);
        }
    }
}
