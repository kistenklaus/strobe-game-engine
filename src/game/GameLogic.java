package game;

import org.joml.Vector3f;

import engine.Logic;
import engine.gfx.Loader;
import engine.gfx.entity.Entity;
import engine.gfx.entity.Light;
import engine.gfx.models.RawModel;
import engine.gfx.models.TexturedModel;
import engine.gfx.models.objLoader.OBJLoader;
import engine.gfx.textures.ModelMaterial;
import game.terrrain.TerrainMap;
import game.terrrain.TerrainRenderer;
import game.terrrain.TerrainShader;

public class GameLogic extends Logic {
	private static final int WIDTH = 1280, HEIGHT = 960;
	private static final float FOV = 70, NEAR_PLANE = 0.1f, FAR_PLANE = 1000f;
	
	public GameLogic() {super(WIDTH, HEIGHT, "Rubik's Cube");}
	
	private EntityRenderer renderer;
	private TerrainRenderer terrainRenderer;
	private Camera camera;
	
	private Entity entity;
	private TerrainMap terrainMap;
	
	@Override
	public void init(Loader loader) {
		loader.createProjMatrix(super.getWidth(), super.getHeight(), FOV, NEAR_PLANE, FAR_PLANE);
		
		this.camera = new Camera(new Vector3f(0,10,100));
		super.engine.setInput(new InputHandler(camera));
		
		Light light = new Light(new Vector3f(2000,2000,2000), new Vector3f(1,0.77f,0.56f));
		
		EntityShader shader = new EntityShader(loader.getProjMatrix());
		this.renderer = new EntityRenderer(shader, light, camera);
		super.engine.addRenderer(renderer);
		
		TerrainShader terrainShader = new TerrainShader(loader.getProjMatrix());
		this.terrainRenderer = new TerrainRenderer(terrainShader, light, camera);
		this.engine.addRenderer(terrainRenderer);
		this.engine.addRenderer(renderer);
		
		
		ModelMaterial entityMaterial = new ModelMaterial(loader.loadTexture("./res/wBlank.png"));
		entityMaterial.setShineDamper(10);
		entityMaterial.setReflectivity(0.2f);
		RawModel rawModel = OBJLoader.loadObjModel("./res/dragon.obj", loader);
		TexturedModel tex_model = new TexturedModel(rawModel, entityMaterial);
		this.entity = new Entity(tex_model, new Vector3f(0,0,-20), 5, 0, 0, 5);
		
		ModelMaterial terrainMaterial = new ModelMaterial(loader.loadTexture("./res/stonebrick_wall.png"));
		this.terrainMap = new TerrainMap(-5, -5, 10, 10, loader, terrainMaterial);
	}

	@Override
	public void repaint() {
		entity.rotateY(1);
		this.renderer.processEntity(entity);
		this.terrainRenderer.processTerrain(terrainMap.getTerrains());
	}

	@Override
	public void terminate() {
		renderer.cleanUp();
	}
}
