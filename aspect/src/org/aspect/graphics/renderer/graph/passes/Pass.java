package org.aspect.graphics.renderer.graph.passes;

import org.aspect.graphics.renderer.graph.Sink;
import org.aspect.graphics.renderer.graph.Source;

import java.util.ArrayList;
import java.util.List;

public abstract class Pass {

    public final String name;

    private List<Sink<?>> sinks = new ArrayList<>();
    private List<Source<?>> sources = new ArrayList<>();

    public Pass(String name) {
        this.name = name;
    }

    //getting around module visibility
    public abstract void complete();


    public abstract void execute();

    public void reset(){}

    protected void registerSink(Sink sink) {
        sink.registerTo(getName());
        sinks.add(sink);
    }

    protected void registerSource(Source source) {
        source.registerTo(getName());
        sources.add(source);
    }

    public List<Sink<?>> getSinks(){
        return sinks;
    }

    public List<Source<?>> getSources(){
        return sources;
    }

    public String getName() {
        return name;
    }

}
