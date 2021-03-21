#pragma once

#include "Hazel/Hazel.h"
#include "Hazel/Core.h"
#include "imgui/imgui.h"

#include "Hazel/Ocean/Ocean.h"
#include "Hazel/ECS/Coordinator.h"
#include "Hazel/ECS/Systems/RenderSystem.h"
#include "Hazel/ECS/Systems/PhysicsSystem.h"
#include "Hazel/ECS/Systems/BoatSystem.h"
#include "Hazel/ECS/ResourceManager.h"
#include "Hazel/Entity/VisibleWorld.h"

#include "/Users/mart_/Desktop/dev/Hazel/Sandbox/src/GameObjects/BoatController.h"

class AtSeaTest : public Hazel::Layer
{
public:
	AtSeaTest();

	void OnUpdate(Hazel::Timestep ts) override;
	virtual void OnImGuiRender() override;
	void OnEvent(Hazel::Event& e) override;
	bool OnKeyPressed(Hazel::KeyPressedEvent& e);
private:
	Hazel::CameraController m_CameraController;
	Hazel::Coordinator m_Coordinator;
	Hazel::Ocean m_Ocean;

	std::unique_ptr<Hazel::ResourceManager> m_ResourceManager = std::make_unique<Hazel::ResourceManager>();
	Hazel::Ref<Hazel::RenderSystem> m_RenderSystem;
	//Hazel::Ref<Hazel::PhysicsSystem> m_PhysicsSystem;
	Hazel::Ref<Hazel::BoatSystem> m_BoatSystem;
	Hazel::VisibleWorld m_VisibleWorld;
	
	BoatController m_BoatController;
};