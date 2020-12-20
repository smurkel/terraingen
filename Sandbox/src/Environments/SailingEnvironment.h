#pragma once

#include "Hazel/Hazel.h"
#include "imgui/imgui.h"

#include "Hazel/Ocean/Ocean.h"
#include "Hazel/Ocean/WaterPhysicsEngine.h"
#include "Hazel/Entity/Island.h"
#include "Hazel/Entity/VisibleWorld.h"
#include "Hazel/Experimental/Terrain.h"

class SailingEnvironment : public Hazel::Layer
{
public:
	SailingEnvironment();

	void OnUpdate(Hazel::Timestep ts) override;
	virtual void OnImGuiRender() override;
	void OnEvent(Hazel::Event& e) override;
private:
	bool Running = false;
	bool OnKeyPressed(Hazel::KeyPressedEvent& e);

	Hazel::CameraController m_CameraController;

	Hazel::Ocean m_Ocean;
	Hazel::Island m_Island;

	Hazel::Library m_Library;

	Hazel::WaterPhysicsEngine m_WPE;
	Hazel::VisibleWorld m_VisibleWorld;
	// 200925
	glm::vec4 c_Emissive;
	glm::vec4 c_Diffuse;
	glm::vec4 c_Ambient;
	glm::vec4 c_Specular;
};