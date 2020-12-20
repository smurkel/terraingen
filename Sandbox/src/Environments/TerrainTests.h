#pragma once

#include "Hazel/Hazel.h"
#include "imgui/imgui.h"

#include "Hazel/Experimental/Isle.h"
#include "Hazel/Entity/VisibleWorld.h"
class TerrainTests : public Hazel::Layer
{
public:
	TerrainTests();

	void OnUpdate(Hazel::Timestep ts) override;
	virtual void OnImGuiRender() override;
	void OnEvent(Hazel::Event& e) override;
private:
	bool OnKeyPressed(Hazel::KeyPressedEvent& e);
	void OceanImGuiSettings();
	
	Hazel::CameraController m_CameraController;
	Hazel::Library m_Library;

	Hazel::Ocean m_Ocean;
	Hazel::Isle m_Isle = Hazel::Isle::Isle();

	Hazel::VisibleWorld m_VisibleWorld;
	// isle imgui settings
	void IsleImGuiSettings();
	bool toggleCoral = false;
	bool toggleDry = false;
	bool toggleErode = false;
	bool toggleGrow = false;
	bool toggleSlide = false;
	int iterCoral = 0;
	int iterDry = 0;
	int iterErode = 0;
	int iterGrow = 0;
	int iterSlide = 0;

	int repeats = 1;
	
};