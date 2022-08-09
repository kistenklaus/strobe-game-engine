package game;

import game.scenes.LevelScene;
import strb.core.Strobe;

public class Main {
	
	public static void main(String[] args) {
		LevelScene levelScene = new LevelScene();
		Strobe.start(levelScene);
	}
	
}
