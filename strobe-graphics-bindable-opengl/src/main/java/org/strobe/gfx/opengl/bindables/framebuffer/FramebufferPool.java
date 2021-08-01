package org.strobe.gfx.opengl.bindables.framebuffer;

import org.strobe.gfx.*;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.Stack;

public class FramebufferPool extends BindablePool<Framebuffer> {

    private final ArrayList<Framebuffer> fbos = new ArrayList<>();

    private final FramebufferBinding binding;

    public FramebufferPool() {
        super(new FramebufferBinding());
        binding = (FramebufferBinding) method;
    }

    public void add(Framebuffer fbo){
        this.fbos.add(fbo);
    }

    protected void rebind(Graphics gfx){
        binding.rebind(gfx);
    }

    public Iterator<Framebuffer> iterator() {
        return fbos.iterator();
    }

    private static class FramebufferBinding implements BindingMethod<Framebuffer> {

        private final Stack<Framebuffer> stack = new Stack<>();

        public FramebufferBinding(){

        }

        @Override
        public void bind(Graphics gfx, Framebuffer fbo) {
            if(fbo.getID() == 0 && stack.isEmpty())return;
            if(!stack.isEmpty() && stack.peek().getID() == fbo.getID())return;
            stack.push(fbo);
            fbo.bind(gfx);
        }

        public void rebind(Graphics gfx){
            if(stack.isEmpty()){
                Framebuffer.getBackBuffer(gfx).bind(gfx);
            }else{
                stack.peek().bind(gfx);
            }
        }

        @Override
        public void unbind(Graphics gfx, Framebuffer fbo) {
            //back buffer case
            if(stack.isEmpty()){
                if(fbo.getID() == 0)return;
                Framebuffer.getBackBuffer(gfx).bind(gfx);
            }else{
                Framebuffer curr = stack.pop();
                if(curr.getID() != fbo.getID()){
                    System.out.println("WARNING : calling unbind on an not bound framebuffer");
                    return;
                }
                if(stack.isEmpty()){
                    Framebuffer.getBackBuffer(gfx).bind(gfx);
                }else{
                    stack.peek().bind(gfx);
                }
            }
        }
    }

}
