package strb.core;

import static org.lwjgl.glfw.GLFW.glfwInit;
import static org.lwjgl.glfw.GLFW.glfwPollEvents;
import static org.lwjgl.glfw.GLFW.glfwSwapInterval;
import org.lwjgl.glfw.GLFWErrorCallback;
import strb.event.windowevents.KeyListener;
import strb.event.windowevents.MouseListener;
import strb.gfx.Window;
import strb.gfx.renderJobs.RenderJob;
import strb.io.IOJob;
import strb.logic.Scene;
import strb.logic.adapter.*;
import strb.multithreading.*;
import strb.util.IntervallTimer;

public final class Engine{
	
	//Reading
	private Thread ioThread;
	private Runnable ioRunnable;
	private JobHandler<IOJob<?>> ioJobHandler;
	private boolean processingIO = false;
	//GFX
	private Thread renderThread;
	private Runnable renderRunnable;
	private Window window;
	private JobHandler<Job> gfxJobHandler;
	private JobHandler<RenderJob> renderJobHandler;
	private JobHandler<RenderJob> loadingScreenRenderJobHandler;
	private boolean rendering = false;
	//Logic
	private Thread logicThread;
	private Runnable logicRunnable;
	private Scene scene;
	private boolean hasNewFrame = true;
	private VAOAdapter vaoAdapter;
	private ShaderAdapter shaderAdapter;
	private TextureAdapter textureAdapter;
	private UBOAdapter uboAdapter;
	private FramebufferAdapter fboAdapter;
	private boolean updating = false;
	private boolean premake = true;
	//Event
	private KeyListener keyListener = new KeyListener();
	private MouseListener mouseListener = new MouseListener();

	//Engine
	private boolean running = false;
	
	public Engine(Scene scene) {
		this.scene = scene;
		this.scene.setEngine(this);
	}

	private void initalizeLogicThread() throws InterruptedException {
		Console.log("Logic-Thread started");
		
		shaderAdapter = new ShaderAdapter(this);
		vaoAdapter = new VAOAdapter(this);
		textureAdapter = new TextureAdapter(this);
		uboAdapter = new UBOAdapter(this);
		fboAdapter = new FramebufferAdapter(this);
		
		scene.preload();
		//waits for every pending Job added in premake
		waitForPendingJobs();
		
		scene.start();
		
		//waits for every pending Job added in scene
		waitForPendingJobs();
		
		shaderAdapter.exitPremak();
		vaoAdapter.exitPremake();
		textureAdapter.exitPremake();
		uboAdapter.exitPremake();
		fboAdapter.exitPremake();
		
		premake = false;
		loadingScreenRenderJobHandler = null;      
		
		Console.log("Scene started");
	}
	
	public void logicLoop() throws InterruptedException {
		
		IntervallTimer timer = new IntervallTimer(0, (s, dt)->{
			scene.updateECS(dt);
			scene.update(dt);
		});
		
		timer.reset();
		while(running) {
			
			updating = true;
			timer.update();
			updating =false;
			
			
			synchronized (logicRunnable) {
				while(hasNewFrame) {
					logicRunnable.wait();
				}
			}
			synchronized (renderRunnable) {
				hasNewFrame = true;
				renderRunnable.notify();
			}
			
		}
		Console.log("Logic-Thread stoped");
	}
	
	private void initalizeRenderThread() {
		Console.log("Render-Thread started");
		initalizeGLFW();
		window = new Window("Strobe", 800, 600, false);
		window.addMouseListener(mouseListener);
		window.addKeyListener(keyListener);
		
		glfwSwapInterval(0);
		
		gfxJobHandler = new JobHandler<Job>();
		renderJobHandler = new JobHandler<RenderJob>();
		loadingScreenRenderJobHandler = new JobHandler<RenderJob>();
	}
	
	public void renderLoop() throws InterruptedException{
		while(!window.shouldClose()) {
			
			//waiting for the next Frame from the Logic
			if(!premake) {
				synchronized (renderRunnable) {
					while(!hasNewFrame) {
						renderRunnable.wait();
					}
				}
			}
			
			OperationQueue<Job> gfxJobs = gfxJobHandler.updateJobs();
			OperationQueue<RenderJob> renderJobs;
			if(premake && loadingScreenRenderJobHandler!=null) {
				renderJobs = loadingScreenRenderJobHandler.updateJobs();
			}else {
				renderJobs = renderJobHandler.updateJobs();
			}
			
			
			//notify the logic that a new Frame should be rendered
			if(!premake) {
				synchronized (logicRunnable) {
					hasNewFrame = false;
					logicRunnable.notify();
				}
			}
			
			rendering = true;
			glfwPollEvents();

			for(Job job : gfxJobs) {
				job.execute();
			}
			gfxJobs.clear();
			
			for(RenderJob job : renderJobs) {
				job.execute();
			}
			
			window.swapBuffers();
			
			rendering = false;
			
		}
		Console.log("Render-Thread stoped");
		stop();
	}
	
	private void inializeIOThread() {
		Console.log("IO-Thread started");
		ioJobHandler = new JobHandler<IOJob<?>>();
	}


	private void IOLoop() throws InterruptedException {
		while(running) {
			synchronized (ioRunnable) {
				processingIO = false;
				if(ioJobHandler.updateJobs().isEmpty()) {
					ioRunnable.wait();
				}
			}
			processingIO = true;
			//processing of IOJobs
			for(IOJob<?> job : ioJobHandler.updateJobs()) {
				job.execute();
			}
			ioJobHandler.clearJobs();
			
			
		}
		Console.log("IO-Thread stoped");
	}

	public void addRenderJob(RenderJob renderJob, int priority) {
		renderJobHandler.addJob(renderJob, priority);
	}
	
	public void addLoadingScreenRenderJob(RenderJob renderJob, int priority) {
		loadingScreenRenderJobHandler.addJob(renderJob, priority);
	}
	
	public void removeRenderJob(RenderJob renderJob) {
		renderJobHandler.removeJob(renderJob);
	}
	
	public void addGfxJob(Job gfxJob, int priority) {
		gfxJobHandler.addJob(gfxJob, priority);
	}
	
	
	public void addIOJob(IOJob<?> ioJob, int priority) {
		ioJobHandler.addJob(ioJob, priority);
		synchronized (ioRunnable) {
			ioRunnable.notify();
		}
	}
	
	public void start() {
		running = true;
		System.out.println("Engine started");
		//Render Thread will be initalized before logic Thread (wait/notify)
		startIOThread();
		startLogicThread();
		startRenderThread();
	}
	
	public void stop() throws InterruptedException{
		running = false;
		synchronized (logicRunnable) {
			hasNewFrame = false;
			logicRunnable.notify();
		}
		logicThread.join();
		synchronized (ioRunnable) {
			ioRunnable.notify();
		}
		ioThread.join();
		Console.log("Engine stoped");
	}

	public void startRenderThread(){
		renderRunnable = new Runnable() {
			@Override
			public void run(){
				Engine.this.initalizeRenderThread();
				synchronized (logicRunnable) {
					logicRunnable.notify();
				}
				try {
					Engine.this.renderLoop();
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
			}
		};
		renderThread = new Thread(renderRunnable);
		renderThread.start();
	}
	
	

	public void startLogicThread() {
		logicRunnable = new Runnable() {
			@Override
			public void run() {
				try {
					synchronized (logicRunnable){
						//waits for the renderThread to be initalized (notified)
						wait();
						Engine.this.initalizeLogicThread();
					}
					Engine.this.logicLoop();
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
			}
		};
		logicThread = new Thread(logicRunnable);
		logicThread.start();
	}
	
	public void startIOThread() {
		ioRunnable = new Runnable() {
			@Override
			public void run() {
				Engine.this.inializeIOThread();
				try {
					Engine.this.IOLoop();
					
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
			}
		};
		ioThread = new Thread(ioRunnable);
		ioThread.start();
	}
	public static void initalizeGLFW() {
		GLFWErrorCallback.createPrint(System.err).set();
		if(!glfwInit()) {
			Console.error("Failed to inialize GLFW");
		}
	}
	/**
	 * 
	 * Getters and Setters
	 * 
	 */

	public ShaderAdapter getShaderBuilder() {
		return shaderAdapter;
	}
	
	public VAOAdapter getVAOBuilder() {
		return vaoAdapter;
	}

	public TextureAdapter getTextureBuilder() {
		return textureAdapter;
	}
	
	public UBOAdapter getUboBuilder() {
		return uboAdapter;
	}
	public FramebufferAdapter getFboAdapter() {
		return fboAdapter;
	}
	
	public boolean isProcessingIO() {
		return processingIO;
	}

	public boolean isRendering() {
		return rendering;
	}

	public boolean isUpdating() {
		return updating;
	}
	
	public KeyListener getKeyListener() {
		return keyListener;
	}
	
	public MouseListener getMouseListener() {
		return mouseListener;
	}
	
	public void waitForPendingJobs() throws InterruptedException {
		shaderAdapter.waitForPendingJobs();
		vaoAdapter.waitForPendingJobs();
		textureAdapter.waitForPendingJobs();
		uboAdapter.waitForPendingJobs();
		fboAdapter.waitForPendingJobs();
	}



	





	
}
