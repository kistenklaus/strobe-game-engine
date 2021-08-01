package org.strobe.debug;

import java.util.ArrayList;

public final class MasterDebugger {

    private final ArrayList<Debugger> debuggers = new ArrayList<>();

    public void addDebugger(Debugger debugger){
        this.debuggers.add(debugger);
    }

    public void debug(Debuggable debuggable){
        for(Debugger debugger : debuggers)debugger.debug(debuggable);
    }

    public void render(){
        for(Debugger debugger : debuggers)debugger.render();
    }

    public void dispose(){
        for(Debugger debugger : debuggers)debugger.dispose();
    }
}
