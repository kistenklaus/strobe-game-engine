package org.aspect.graphics.drawable.attachments;

import org.aspect.graphics.bindable.Bindable;
import org.aspect.graphics.drawable.Attachment;

public abstract class BindableAttachment<T extends Bindable> implements Attachment {

    private final T bindable;

    public BindableAttachment(T bindable){
        this.bindable = bindable;
    }

    public void bind(){
        bindable.bind();
    }

    public T getBindable(){
        return bindable;
    }

    public void unbind(){
        bindable.unbind();
    }
}
