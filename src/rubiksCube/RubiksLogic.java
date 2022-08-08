package rubiksCube;

import org.joml.Vector3f;

import engine.Logic;
import engine.gfx.Loader;
import engine.gfx.entity.Entity;
import engine.gfx.entity.Light;
import engine.gfx.models.OBJLoader;
import engine.gfx.models.RawModel;
import engine.gfx.models.TexturedModel;
import engine.gfx.renderer.Renderer;
import engine.gfx.shaders.Shader3D;
import engine.gfx.textures.ModelMaterial;

public class RubiksLogic extends Logic {
	private static final int WIDTH = 640, HEIGHT = 480;
	private static final float FOV = 70, NEAR_PLANE = 0.1f, FAR_PLANE = 1000f;
	
	public RubiksLogic() {super(WIDTH, HEIGHT, "Rubik's Cube");}
	
	private Entity entity;
	private Renderer renderer;
	private Camera camera;
	
	@Override
	public void init(Loader loader) {
		loader.createProjMatrix(super.getWidth(), super.getHeight(), FOV, NEAR_PLANE, FAR_PLANE);
		this.camera = new Camera(new Vector3f(0,0,10));
		super.engine.setInput(new InputHandler(camera));
		Light light = new Light(new Vector3f(2000,2000,2000), new Vector3f(1,1,1));
		ModelMaterial material = new ModelMaterial(loader.loadTexture("./res/wBlank.png"));
		material.setShineDamper(10);
		material.setReflectivity(0.2f);
		RawModel rawModel = OBJLoader.loadObjModel("./res/dragon.obj", loader);
		TexturedModel tex_model = new TexturedModel(rawModel, material);
		Shader3D shader = new Shader3Dexample(loader.getProjMatrix());
		this.renderer = new Renderer3D(shader, light, camera);
		this.entity = new Entity(tex_model, new Vector3f(0,-8,-20), 5, 0, 0, 2);
		this.engine.addRenderer(renderer);
	}

	@Override
	public void repaint() {
		entity.rotateY(1);
		this.renderer.processEntity(entity);
	}

	@Override
	public void terminate() {
		renderer.cleanUp();
	}
}
