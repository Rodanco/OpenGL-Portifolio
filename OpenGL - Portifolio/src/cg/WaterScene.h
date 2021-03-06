#pragma once
#include "../Scene.h"
#include "../Camera.h"
#include "../Keyboard.h"
#include "../Shader.h"
#include "../Phong/DirectionalLight.h"
#include "../Phong/PhongMaterial.h"
#include "../postFx/PostFXMaterial.h"
#include "../Phong/WaterMaterial.h"
#include "../Phong/SkyMaterial.h"
#include "MeshFactory.h"


class WaterScene : public Scene
{
	Keyboard *keys = Keyboard::getInstace();
	Mesh* terrain, *skydome;
	Camera *camera;
	Matrix4 *world;
	PhongMaterial* terrainMaterial;
	SkyMaterial* skydomeMaterial;
	Light* light;
	int textureRepeat = 10;
	glm::vec2 cloud1Offset = glm::vec2(.001f, .005f);
	glm::vec2 cloud2Offset = glm::vec2(-.01f, .001f);

	float WATER_H = 11.0f;
	Mesh* water;
	std::unique_ptr<WaterMaterial> waterMaterial;

	Mesh* canvas;
	FrameBuffer* fb;
	PostFXMaterial* postFX;

	float percent = 0.f;
	float sign = 1.f;

public:


	// Inherited via Scene
	virtual void init() override
	{
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glPolygonMode(GL_FRONT_FACE, GL_LINE);
		world = new Matrix4(glm::mat4());
		camera = new Camera(glm::vec3(145.f, 20.f, 0.f));
		light = new DirectionalLight(
			glm::vec3(1.f, -1.f, -1.f),
			glm::vec3(.1f, .1f, .1f),
			glm::vec3(1.f, 1.f, 1.f),
			glm::vec3(1.f, 1.f, 1.f)
		);

		//Terreno
		terrain = MeshFactory::LoadTerrain("res/Images/river.jpg", .4f, textureRepeat);
		terrainMaterial = new PhongMaterial(
			glm::vec3(1.f, 1.f, 1.f),
			glm::vec3(.9f, .9f, .9f),
			glm::vec3(.0f, .0f, .0f),
			.0f);
		terrainMaterial->setTexture(new Texture("res/Textures/snow.jpg"))
			->setTexture(new Texture("res/Textures/rock.jpg"))
			->setTexture(new Texture("res/Textures/grass.jpg"))
			->setTexture(new Texture("res/Textures/sand.jpg"))
			->setTexture("uBlendMapTexture", new Texture("res/Textures/BlendMap.png"));
		terrain->setUniform("uWorld", world);
		terrain->setUniform("uTexRepeat", new Int(textureRepeat));

		//Skydome
		skydome = MeshFactory::createSkydome(10, 10, 1);
		skydome->setUniform("uWorld", new Matrix4(glm::scale(glm::mat4(), glm::vec3(18000.f, 1000.f, 18000.f))));
		skydomeMaterial = new SkyMaterial();
		skydomeMaterial->setTexture("uTexCloud1", new Texture("res/Textures/cloud1.jpg"))
			->setTexture("uTexCloud2", new Texture("res/Textures/cloud2.jpg"));


		//Agua
		water = MeshFactory::createXZSquare(400, 300, WATER_H);
		water->setUniform("uWorld", new Matrix4(glm::scale(glm::mat4(), glm::vec3(100.f, 1.f, 100.f))));
		waterMaterial = std::make_unique<WaterMaterial>();

		//Canvas para o postFX
		canvas = MeshFactory::createCanvas();
		fb = FrameBuffer::forCurrentViewport();
		//postFX = PostFXMaterial::defaultPostFX(fb);
		postFX = new PostFXMaterial("fxSharpen", fb);
	}

	virtual void update(float secs) override
	{
		if (keys->isDown(GLFW_KEY_W))
			camera->moveFront(secs * 20);
		else if (keys->isDown(GLFW_KEY_S))
			camera->moveFront(-secs * 20);

		if (keys->isDown(GLFW_KEY_A))
			camera->rotateYaw(secs);
		else if (keys->isDown(GLFW_KEY_D))
			camera->rotateYaw(-secs);

		if (keys->isDown(GLFW_KEY_RIGHT))
			camera->strafeRight(-secs * 20);
		else if (keys->isDown(GLFW_KEY_LEFT))
			camera->strafeRight(secs * 20);

		if (keys->isDown(GLFW_KEY_UP))
			camera->rotatePitch(secs);
		else if (keys->isDown(GLFW_KEY_DOWN))
			camera->rotatePitch(-secs);

		const float mult = .008f;
		cloud1Offset += secs * mult;
		cloud2Offset += secs * mult;
		skydomeMaterial->setOffset("uTexOffset1", cloud1Offset)->setOffset("uTexOffset2", cloud2Offset);

		percent += secs * sign;
		if (percent >= 1.f)
			sign = -1.f;
		else if (percent <= 0.f)
			sign = 1.f;
		postFX->updatePercentage(percent);
	}

	void drawScene()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		drawSky();
		drawTerrain();
		drawWater();
	}

	virtual void draw() override
	{
		fb->bind();
		drawScene();
		fb->unbind();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		canvas->draw(postFX);
	}

	void drawSky()
	{
		glDisable(GL_DEPTH_TEST);

		skydomeMaterial->getShader()
			->bind()
			->setUniform("uProjection", camera->getProjectionMatrix())
			->setUniform("uView", camera->getViewMatrix())
			->unbind();
		skydome->draw(skydomeMaterial);

		glEnable(GL_DEPTH_TEST);
	}

	void drawTerrain()
	{
		Shader* shader = terrainMaterial->getShader();
		shader->bind()->set(camera)->set(light)->unbind();
		terrain->draw(terrainMaterial);
	}

	void drawWater()
	{
		waterMaterial->getShader()->bind()->set(camera)->unbind();
		water->draw(waterMaterial.get());
	}

	virtual void deinit() override
	{
		delete terrainMaterial;
		delete skydomeMaterial;
		delete light;
		delete terrain;
		delete skydome;
		delete camera;
		delete world;
	}
};