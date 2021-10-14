package org.strobe.engine.development.profiler;

import java.util.HashMap;
import java.util.Set;

public final class ProfilerCompleteFrame {

    private final float start;
    private final float end;
    private final float duration;
    private final String name;

    private final HashMap<String, ProfilerCompleteFrame> children;

    public ProfilerCompleteFrame(ProfilerFrame frame, long parentStart, long parentDuration){
        this.name = frame.getName();
        start = Math.min(1,Math.max(0,(float)((frame.getStart() - parentStart)/(double)parentDuration)));
        end = Math.min(1, Math.max(0,(float)((frame.getEnd() - parentStart)/(double)parentDuration)));
        duration = frame.getDuration()*1e-6f;

        this.children = new HashMap<>();
        for(ProfilerFrame child : frame.children()){
            this.children.put(child.getName(), new ProfilerCompleteFrame(child, frame.getStart(), frame.getDuration()));
        }
    }

    public ProfilerCompleteFrame(ProfilerFrame frame){
        this(frame, frame.getStart(), frame.getDuration());
    }

    public ProfilerCompleteFrame(String name, float start, float end, float duration, HashMap<String, ProfilerCompleteFrame> children){
        this.name = name;
        this.start = Math.min(1, Math.max(0, start));
        this.end = Math.min(1, Math.max(0,end));
        this.duration = duration;
        this.children = children;
    }

    public float getStart() {
        return start;
    }

    public float getEnd() {
        return end;
    }

    public float getDuration() {
        return duration;
    }

    public String getName() {
        return name;
    }

    public Iterable<ProfilerCompleteFrame> children(){
        return children.values();
    }

    public Set<String> getChildNames(){
        return children.keySet();
    }

    public ProfilerCompleteFrame getChild(String name){
        return children.get(name);
    }

}
