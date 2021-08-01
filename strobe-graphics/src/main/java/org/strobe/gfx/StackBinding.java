package org.strobe.gfx;

import java.util.Stack;

public class StackBinding implements BindingMethod<Bindable>{

    protected Stack<Bindable> stack = new Stack<>();

    @Override
    public void bind(Graphics gfx, Bindable bindable) {
        if(!stack.isEmpty() && stack.peek().equals(bindable))return;
        bindable.bind(gfx);
        stack.push(bindable);
    }

    public void rebind(Graphics gfx){
        Bindable current = stack.peek();
        if(current != null)current.bind(gfx);
    }
    @Override
    public void unbind(Graphics gfx, Bindable bindable) {
        if(stack.isEmpty())return;
        if(!stack.peek().equals(bindable))return;
        stack.pop();
        //bind the prev bound bindable
        if(stack.isEmpty())return;
        stack.peek().bind(gfx);
    }
}
