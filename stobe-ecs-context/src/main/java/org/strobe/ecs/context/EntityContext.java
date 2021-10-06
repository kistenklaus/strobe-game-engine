package org.strobe.ecs.context;

import org.strobe.ecs.EntityComponentSystem;
import org.strobe.ecs.context.renderer.EntityRenderer;
import org.strobe.engine.StrobeContext;
import org.strobe.gfx.Graphics;
import org.strobe.window.WindowKeyboard;
import org.strobe.window.WindowMouse;
import org.strobe.ecs.context.scripts.ScriptSystem;

public abstract class EntityContext extends StrobeContext {

    private EntityRenderer renderer = null;
    private WindowMouse mouse = null;
    private WindowKeyboard keyboard = null;
    protected final EntityComponentSystem ecs = new EntityComponentSystem();

    public EntityContext(String title, int width, int height){
        super(title, width, height);
    }


    @Override
    public void init(Graphics gfx) {
        renderer = new EntityRenderer(gfx, ecs);
        gfx.addRenderer(0, renderer);

        mouse = gfx.getWindow().getMouse();
        keyboard = gfx.getWindow().getKeyboard();

        setup(gfx);

        ecs.addEntitySystem(new ScriptSystem(ecs));
    }


    public abstract void setup(Graphics gfx);



    @Override
    public void render(Graphics gfx) {
    }

    @Override
    public void update(float dt) {
        ecs.update(dt);
    }

    public final WindowMouse getMouse(){
        return mouse;
    }

    public final WindowKeyboard getKeyboard(){
        return keyboard;
    }

}
