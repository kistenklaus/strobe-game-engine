package strb.io;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Paths;
import strb.core.Console;

public class ReadFileJob extends IOJob<String>{

	private final String FILEPATH;
	
	public ReadFileJob(String filepath) {
		FILEPATH = filepath;
	}
	
	@Override
	protected void perform() {
		try {
			result = new String(Files.readAllBytes(Paths.get(FILEPATH)));
		} catch (IOException e) {
			result = null;
			e.printStackTrace();
		}
	}
	
}
