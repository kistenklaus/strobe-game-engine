package org.strobe.engine.development.ui;

import imgui.ImGui;
import imgui.ImVec2;
import imgui.flag.ImGuiCol;
import org.strobe.engine.development.ProxyPrintStream;
import org.strobe.gfx.Graphics;

import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;
import java.util.LinkedList;

public final class ConsolePanel implements UIPanel {

    private static final int CONSOLE_MAX_LINES = 100;
    private static final float LINE_HEIGHT = 20;
    private final LinkedList<ConsoleLine> consoleLines = new LinkedList<>();
    private boolean scrollDown = false;

    private final DevelopmentStyle style;

    public ConsolePanel(ProxyPrintStream printStream, DevelopmentStyle style) {
        printStream.setOut(this::consoleInput);
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

}
