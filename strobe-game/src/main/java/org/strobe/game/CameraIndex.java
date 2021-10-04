package org.strobe.game;

import org.strobe.ecs.Component;

public final class CameraIndex implements Component {

    private final int index;

    public CameraIndex(int index){
        this.index = index;
    }

    public int getIndex() {
        return index;
    }
}
