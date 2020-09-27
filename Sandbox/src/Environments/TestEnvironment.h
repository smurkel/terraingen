#pragma once

#include "Hazel/Hazel.h"
#include "imgui/imgui.h"

#include "Hazel/Ocean/Ocean.h"
#include "Hazel/Ocean/WaterPhysicsEngine.h"
#include "Hazel/Entity/Boat.h"
#include "Hazel/Entity/VisibleWorld.h"
#include "Hazel/Experimental/Terrain.h"

class TestEnvironment : public Hazel::Layer
{
public:
	TestEnvironment();

	void OnUpdate(Hazel::Timestep ts) override;
	virtual void OnImGuiRender() override;
	void OnEvent(Hazel::Event& e) override;
private:
	bool Running = false;
	bool OnKeyPressed(Hazel::KeyPressedEvent& e);

	Hazel::CameraController m_CameraController;

	Hazel::Ocean m_Ocean;
	Hazel::Boat m_Boat;

	Hazel::Library m_Library;
	
	Hazel::WaterPhysicsEngine m_WPE;
	Hazel::VisibleWorld m_VisibleWorld;
	// 200925
	glm::vec4 c_Emissive;
	glm::vec4 c_Diffuse;
	glm::vec4 c_Ambient;
	glm::vec4 c_Specular;
	// 200923:
	int N = 256;
	float scale = 5;
	float gridsize = 100;
	float height = 5.0;
	int octaves = 12;
	float persistence = 0.3;
	float lacunarity = 2.5;
	int seed = 0;
	Hazel::Terrain m_Terrain = Hazel::Terrain(seed, N, gridsize, scale, height, octaves, persistence, lacunarity);
	bool keepSeed = false;
	// erosion settings
	int ITERATIONS = 1;
	float VELOCITY = 1.0;
	float VOLUME = 1.0;
	float INERTIA = 0.4;
	float CAPACITY = 8.0;
	float EROSION = 0.5;
	float GRAVITY = 10.0;
	float EVAPORATION = 0.0125;
	float DEPOSITION = 0.1;
	int MINSLOPE_EXP = -3;
	float MINSLOPE = 0.001;
	float WATERLEVEL = -1.0;
	bool INVERTEROSION = false;
	int MAXSTEPS = 64;
	float ARIDITY = 0.0; 
	float GROWTHSTEEPNESS = 4.0;
};