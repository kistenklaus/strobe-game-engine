package boot;

import engine.Engine;
import game.GameLogic;

public class Boot {
	
	public static void main(String[] args) {
		Engine engine = new Engine(new GameLogic());
		engine.start();
	}
	
}
