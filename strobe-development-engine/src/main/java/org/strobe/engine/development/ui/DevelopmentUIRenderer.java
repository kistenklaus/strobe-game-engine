package org.strobe.engine.development.ui;

import org.strobe.gfx.Graphics;
import org.strobe.gfx.Renderer;

import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;

public final class DevelopmentUIRenderer implements Renderer<UIPanel> {

    private final List<UIPanel> uis = new ArrayList<>();
    private final LinkedList<UIPanel> scheduledAdds = new LinkedList<>();
    private final LinkedList<UIPanel> scheduledRemoves = new LinkedList<>();

    private boolean initalized = false;

    private final DevelopmentStyle style = new DevelopmentStyle();

    public void addUI(UIPanel panel){
        if(!initalized)uis.add(panel);
        else scheduledAdds.add(panel);
    }

    public void removeUI(UIPanel panel){
        if(!initalized)uis.remove(panel);
        else scheduledRemoves.add(panel);
    }

    public void init(Graphics gfx){
        if(initalized)return;
        style.init(gfx);
        for(UIPanel panel : uis){
            panel.init(gfx);
        }
    }

    @Override
    public void render(Graphics gfx) {
        while(!scheduledAdds.isEmpty()){
            UIPanel toAdd = scheduledAdds.pop();
            toAdd.init(gfx);
            uis.add(toAdd);
        }

        while(!scheduledRemoves.isEmpty()){
            UIPanel toRemove = scheduledRemoves.pop();
            toRemove.dispose(gfx);
            uis.remove(toRemove);
        }
        style.push();
        for(UIPanel panel : uis){
            panel.draw(gfx);
        }
        style.pop();
    }

    @Override
    public void dispose(Graphics gfx) {
        for(UIPanel panel : uis){
            panel.dispose(gfx);
        }
        for(UIPanel panel : scheduledRemoves){
            panel.dispose(gfx);
        }
    }

    public DevelopmentStyle getStyle(){
        return style;
    }
}
