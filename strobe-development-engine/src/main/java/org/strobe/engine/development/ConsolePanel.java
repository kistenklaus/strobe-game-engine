package org.strobe.engine.development;

import imgui.ImGui;
import imgui.ImVec2;
import imgui.flag.ImGuiCol;
import jdk.jfr.StackTrace;
import org.strobe.gfx.Graphics;

import java.io.ByteArrayOutputStream;
import java.io.PrintStream;
import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;
import java.util.LinkedList;
import java.util.function.Consumer;

public class ConsolePanel extends Panel{

    private static final int CONSOLE_MAX_LINES = 100;
    private static final float LINE_HEIGHT = 20;
    private final LinkedList<ConsoleLine> consoleLines = new LinkedList<>();
    private boolean scrollDown = false;

    private final DevelopmentStyle style;

    public ConsolePanel(DevelopmentStyle style) {
        new ProxyPrintStream(this::consoleInput);
        this.style = style;
    }

    private void consoleInput(String str){
        //implNote : No System.out.println in this function!!!
        scrollDown = true;
        consoleLines.add(new ConsoleLine(str));
        if(consoleLines.size() > CONSOLE_MAX_LINES)consoleLines.removeFirst();
    }

    @Override
    public void init(Graphics gfx) {

    }

    @Override
    public void draw(Graphics gfx) {
        ImGui.pushStyleColor(ImGuiCol.ChildBg, style.getDarkColor());
        if(ImGui.begin("Console")){
            if(scrollDown){
                ImGui.setScrollY(ImGui.getScrollMaxY());
                scrollDown = false;
            }
            ImVec2 region = ImGui.getContentRegionAvail();

            for(ConsoleLine line : consoleLines){
                ImVec2 cp = ImGui.getCursorPos();
                ImVec2 scp = ImGui.getCursorScreenPos();
                ImGui.setCursorPos(cp.x, cp.y +LINE_HEIGHT/2f-ImGui.getFontSize()/2f);
                ImGui.text(line.str);
                if(ImGui.isMouseHoveringRect(scp.x, scp.y, region.x-scp.x, scp.y+LINE_HEIGHT)){
                    ImGui.setTooltip(line.hint);
                }
                ImGui.setCursorPosY(cp.y+LINE_HEIGHT);
            }
        }
        ImGui.end();
        ImGui.popStyleColor();
    }

    private final class ConsoleLine {
        private static final DateTimeFormatter formatter = DateTimeFormatter.ofPattern("HH:mm:ss");
        private static final int REVERT_DEPTH = 4;
        private final String str;
        private final String hint;
        public ConsoleLine(String s){
            StackTraceElement[] stackTrace = Thread.currentThread().getStackTrace();
            StringBuilder hint = new StringBuilder();
            hint.append(stackTrace[REVERT_DEPTH].getClassName()).append(".")
                    .append(stackTrace[REVERT_DEPTH].getMethodName().replaceAll("<|>", ""))
                    .append("(")
                    .append(stackTrace[REVERT_DEPTH].getFileName()).append(":")
                    .append(stackTrace[REVERT_DEPTH].getLineNumber()).append(")");

            this.hint = hint.toString();

            StringBuilder str = new StringBuilder();
            LocalDateTime now = LocalDateTime.now();
            str.append("[").append(now.format(formatter)).append("]: ")
                    .append(s);

            this.str = str.toString();
        }
    }

    private final class ProxyPrintStream extends PrintStream {

        private final PrintStream prev;
        private final Consumer<String> out;

        public ProxyPrintStream(Consumer<String> out) {
            super(new ByteArrayOutputStream());
            this.out = out;
            prev = System.out;
            System.setOut(this);
        }

        @Override
        public void print(String s) {
            prev.print(s);
            out.accept(s);
        }

        @Override
        public void println(String x) {
            prev.println(x);
            out.accept(x);
        }

        public void close(){
            System.setOut(prev);
            close();
        }


        @Override
        public void print(boolean b) {
            print(Boolean.toString(b));
        }

        @Override
        public void print(char c) {
            print(Character.toString(c));
        }

        @Override
        public void print(int i) {
            print(Integer.toString(i));
        }

        @Override
        public void print(long l) {
            print(Long.toString(l));
        }

        @Override
        public void print(float f) {
            print(Float.toString(f));
        }

        @Override
        public void print(double d) {
            print(Double.toString(d));
        }

        @Override
        public void print(char[] s) {
            print(new String(s));
        }


        @Override
        public void print(Object obj) {
            print(obj.toString());
        }

        @Override
        public void println(boolean x) {
            println(Boolean.toString(x));
        }

        @Override
        public void println(char x) {
            println(Character.toString(x));
        }

        @Override
        public void println(int x) {
            println(Integer.toString(x));
        }

        @Override
        public void println(long x) {
            println(Long.toString(x));
        }

        @Override
        public void println(float x) {
            println(Float.toString(x));
        }

        @Override
        public void println(double x) {
            println(Double.toString(x));
        }

        @Override
        public void println(char[] x) {
            println(new String(x));
        }


        @Override
        public void println(Object x) {
            println(x.toString());
        }
    }

}
