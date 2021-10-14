package org.strobe.engine.development.profiler;

import java.util.*;

public final class Profiler {

    public static final int MAX_HISTORY_LENGTH = 1500;


    private long start = 0;
    private float trueFrameTime = 0;

    private ProfilerCompleteFrame prev = null;

    private final Stack<ProfilerFrame> frameStack = new Stack<>();

    private ProfilerCompleteFrame averageFrame = null;

    private boolean calcHistory = false;
    private boolean calcAvg = false;
    private boolean calcPrev = false;

    private final LinkedList<ProfilerCompleteFrame> history = new LinkedList<>();

    public void startFrame() {
        start = System.nanoTime();
        frameStack.clear();
        frameStack.push(new ProfilerFrame("frame"));
    }

    public void pushFrame(String name) {
        ProfilerFrame frame = new ProfilerFrame(name);
        frameStack.peek().addChild(frame);
        frameStack.push(frame);
    }

    public void popFrame() {
        ProfilerFrame frame = frameStack.pop();
        frame.close();
    }

    public void endFrame() {
        ProfilerFrame frame = frameStack.pop();
        frame.close();

        if(calcPrev){
            ProfilerCompleteFrame completeFrame = new ProfilerCompleteFrame(frame);
            prev = completeFrame;
            //average history!
            if(calcHistory){
                history.push(completeFrame);
                if(history.size()>MAX_HISTORY_LENGTH)history.removeLast();
                if(calcAvg){
                    averageFrame = average(history, frame.getName());
                }else averageFrame = null;
            }else {
                averageFrame = null;
            }
        }else {
            prev = null;
            averageFrame = null;
        }


        trueFrameTime = (System.nanoTime()-start)*1e-9f;
    }

    private ProfilerCompleteFrame average(Collection<ProfilerCompleteFrame> frames, String name) {
        float avgStart = 0;
        float avgEnd = 0;
        float avgDur = 0;
        for (ProfilerCompleteFrame frame : frames) {
            avgStart += frame.getStart();
            avgEnd += frame.getEnd();
            avgDur += frame.getDuration();
        }
        avgStart /= frames.size();
        avgEnd /= frames.size();
        avgDur /= frames.size();


        Set<String> childNames = new HashSet<>();
        for (ProfilerCompleteFrame frame : frames) {
            childNames.addAll(frame.getChildNames());
        }

        HashMap<String, ProfilerCompleteFrame> children = new HashMap<>();
        for (String childName : childNames) {
            ArrayList<ProfilerCompleteFrame> childs = new ArrayList<>();
            for (ProfilerCompleteFrame frame : frames) {
                ProfilerCompleteFrame child = frame.getChild(childName);
                if (child != null) childs.add(child);
            }
            children.put(childName, average(childs, childName));
        }


        return new ProfilerCompleteFrame(name, avgStart, avgEnd, avgDur, children);
    }

    public ProfilerCompleteFrame getPrevFrame() {
        return prev;
    }

    public ProfilerCompleteFrame getAvgFrame(){
        return averageFrame;
    }

    public void enableAvgFrame(){
        enableHistory();
        calcAvg = true;
    }

    public void disableAvgFrame(){
        calcAvg = false;
    }

    public float getTrueFrameTime(){
        return trueFrameTime;
    }

    public void enablePrevFrame(){
        calcPrev = true;
    }

    public void disablePrevFrame(){
        calcPrev = false;
        disableHistory();
    }

    public void enableHistory(){
        enablePrevFrame();
        if(!calcHistory)history.clear();
        calcHistory = true;
    }

    public void disableHistory(){
        disableAvgFrame();
        if(calcHistory)history.clear();
        calcHistory = false;
    }

    public int getHistorySize(){
        return history.size();
    }

    public Iterable<ProfilerCompleteFrame> getHistory() {
        return history;
    }
}
