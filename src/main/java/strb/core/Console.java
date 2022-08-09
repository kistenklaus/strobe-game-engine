package strb.core;


public class Console {
	
	public static void log(Object print) {
		if(print == null)Console.log("null");
		else Console.log(print.toString());
	}
	
	public static void log(String print) {
		System.out.println(print);
	}

	public static void error(String error) {
		error(error, false);
	}
	
	public static void error(String error, boolean throwException) {
		assert !throwException : error;
	}
	
}
