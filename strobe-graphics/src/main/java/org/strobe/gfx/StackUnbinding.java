package org.strobe.gfx;

import java.util.Stack;

public final class StackUnbinding implements BindingMethod<Bindable>{


    private Stack<Bindable> stack = new Stack<>();

    @Override
    public void bind(Graphics gfx, Bindable bindable) {
        if(!stack.isEmpty() && stack.peek().equals(bindable))return;
        //unbind the top
        if(!stack.isEmpty())stack.peek().unbind(gfx);
        //push and bind
        stack.push(bindable);
        bindable.bind(gfx);

    }

    public void rebind(Graphics gfx){
        Bindable current = stack.peek();
        if(current != null)current.bind(gfx);
    }

    @Override
    public void unbind(Graphics gfx, Bindable bindable) {
        if(stack.isEmpty())return;
        if(!stack.peek().equals(bindable))return;
        //unbind the top of the stack
        stack.pop().unbind(gfx);
        //bind the prev bound bindable
        if(stack.isEmpty())return;
        stack.peek().bind(gfx);
    }
}
