#include "TerrainTests.h"
#include "Hazel/Events/Event.h"

TerrainTests::TerrainTests()
	: Layer("Test layer"), m_CameraController(), m_VisibleWorld(m_Library, m_CameraController.GetCamera())
{
	m_VisibleWorld.AddIsle(&m_Isle);
	//m_Isle.Sunlight();

	m_Ocean.Generate(1.0, glm::vec2(7.0, 7.0), 1, true);
	//m_Ocean.SetWaterlevel(m_Isle.WATERLEVEL - m_Isle.p_Height);
	m_Ocean.SetMurkiness(2.0);
	m_VisibleWorld.SetOcean(&m_Ocean);
	
	m_VisibleWorld.SetCamera(m_CameraController.GetCamera());
}

void TerrainTests::OnUpdate(Hazel::Timestep ts)
{
	m_Ocean.Update(ts);
	m_CameraController.OnUpdate(ts);
	m_VisibleWorld.Render();
	//m_Isle.Render(m_CameraController.GetCamera());
	HZ_CORE_INFO("framerate: {0}", 1.0 / ts.GetSeconds());
	
}

void TerrainTests::OnImGuiRender()
{
	OceanImGuiSettings();
	IsleImGuiSettings();
}

void TerrainTests::OnEvent(Hazel::Event& e)
{
	m_CameraController.OnEvent(e);
	Hazel::EventDispatcher dispatcher(e);
	dispatcher.Dispatch<Hazel::KeyPressedEvent>(HZ_BIND_EVENT_FN(TerrainTests::OnKeyPressed));
}

bool TerrainTests::OnKeyPressed(Hazel::KeyPressedEvent& e)
{
	if (e.GetKeyCode() == HZ_KEY_SPACE)
		m_Ocean.TogglePause();
	if (e.GetKeyCode() == HZ_KEY_N)
		m_Ocean.ToggleNormal();
	if (e.GetKeyCode() == HZ_KEY_W)
		m_Isle.WATERLEVEL += 3.0;
	if (e.GetKeyCode() == HZ_KEY_S)
		m_Isle.WATERLEVEL -= 3.0;
	//if (e.GetKeyCode() == HZ_KEY_SPACE)
	//	m_Isle.Erode();
	//if (e.GetKeyCode() == HZ_KEY_B)
	//{
	//	m_Isle.Blur(0);
	//	m_Isle.Blur(1);
	//	m_Isle.Blur(2);
	//}
	//if (e.GetKeyCode() == HZ_KEY_S)
	//{
	//	// get to steady state dryness:
	//	for (int j = 0; j < 20; j++)
	//	{
	//		HZ_CORE_INFO("Getting to steady stage - step #{0}", j);
	//		m_Isle.Erode();
	//		m_Isle.Dry();
	//		Hazel::RenderCommand::SetClearColor(glm::vec4(0.2, 0.5, 0.7, 1.0));
	//		Hazel::RenderCommand::Clear();
	//		Hazel::Renderer::BeginScene(m_CameraController.GetCamera(), glm::vec3(1.0, 1.0, 1.0));

	//		m_Isle.Render(m_CameraController.GetCamera());

	//		Hazel::Renderer::EndScene();
	//	}
	//}
	//if (e.GetKeyCode() == HZ_KEY_P)
	//{
	//	for (int j = 0; j < 20; j++)
	//	{
	//		HZ_CORE_INFO("Simulating 20 years - year: #{0}", j);
	//		m_Isle.Erode();
	//		m_Isle.Grow();
	//		m_Isle.Dry();
	//	}
	//}
	//if (e.GetKeyCode() == HZ_KEY_LEFT)
	//	m_Isle.SwitchDown();
	//if (e.GetKeyCode() == HZ_KEY_RIGHT)
	//	m_Isle.SwitchUp();
	//if (e.GetKeyCode() == HZ_KEY_D)
	//{
	//	m_Isle.Dry();
	//	m_Isle.Blur(1);
	//}
	//if (e.GetKeyCode() == HZ_KEY_G)
	//	m_Isle.Grow();
	//if (e.GetKeyCode() == HZ_KEY_E)
	//	m_Isle.Export();
	//if (e.GetKeyCode() == HZ_KEY_I)
	//{
	//	m_Isle.Sunlight();
	//}
	//if (e.GetKeyCode() == HZ_KEY_EQUAL)
	//	m_Isle.ResolutionDouble();
	//if (e.GetKeyCode() == HZ_KEY_MINUS)
	//	m_Isle.ResolutionHalve();
	//if (e.GetKeyCode() == HZ_KEY_N)
	//{
	//	m_Isle.WATERLEVEL = 0.0f;
	//	m_Isle.GenerateRandom(256);
	//	m_Isle.p_Cellsize = 1.0;
	//}

	//if (e.GetKeyCode() == HZ_KEY_U)
	//{
	//	m_Isle.WATERLEVEL += 1.0f;
	//	m_Ocean.SetWaterlevel(m_Isle.WATERLEVEL);
	//}
	//	
	//if (e.GetKeyCode() == HZ_KEY_Y)
	//{
	//	m_Isle.WATERLEVEL -= 1.0f;
	//	m_Ocean.SetWaterlevel(m_Isle.WATERLEVEL);
	//}

	//if (e.GetKeyCode() == HZ_KEY_J)
	//{
	//	m_Isle.LATITUDE += 10.0;
	//	m_Isle.Sunlight();
	//}

	//if (e.GetKeyCode() == HZ_KEY_H)
	//{
	//	m_Isle.LATITUDE -= 10.0;
	//	m_Isle.Sunlight();
	//}

	//if (e.GetKeyCode() == HZ_KEY_Z)
	//	m_Isle.Slide();
	//if (e.GetKeyCode() == HZ_KEY_T)
	//	m_Isle.ThermalErosion();
	//if (e.GetKeyCode() == HZ_KEY_V)
	//	m_Isle.Erupt();
	//if (e.GetKeyCode() == HZ_KEY_C)
	//	m_Isle.Coral();
	//if (e.GetKeyCode() == HZ_KEY_X)
	//{
	//	for (int i = 0; i < 40; i++)
	//	{
	//		m_Isle.Erupt();
	//		m_Isle.Erupt();
	//		m_Isle.Coral();
	//		m_Isle.Coral();
	//		m_Isle.Coral();
	//		float WL = m_Isle.WATERLEVEL;
	//		m_Isle.WATERLEVEL = 0.0;
	//		m_Isle.Grow();
	//		m_Isle.WATERLEVEL = WL;
	//	}
	//}
	return true;
}

void TerrainTests::OceanImGuiSettings()
{
	ImGui::Begin("Ocean settings");
	float Transparency = m_Ocean.GetMurkiness();
	ImGui::SliderFloat("Water clarity", &Transparency, 0.0, 50.0);
	m_Ocean.SetMurkiness(Transparency);
	glm::vec4 cE = m_Ocean.GetColorVec4(0);
	glm::vec4 cA = m_Ocean.GetColorVec4(1);
	glm::vec4 cD = m_Ocean.GetColorVec4(2);
	glm::vec4 cS = m_Ocean.GetColorVec4(3);
	ImGui::ColorPicker4("Emissive colour", &cE.r);
	ImGui::ColorPicker4("Ambient colour", &cA.r);
	ImGui::ColorPicker4("Diffuse colour", &cD.r);
	ImGui::ColorPicker4("Specular colour", &cS.r);
	m_Ocean.SetColorVec4(0, cE);
	m_Ocean.SetColorVec4(1, cA);
	m_Ocean.SetColorVec4(2, cD);
	m_Ocean.SetColorVec4(3, cS); 
	ImGui::End();
}

void TerrainTests::IsleImGuiSettings()
{
	ImGui::Begin("Terrain settings");
	ImGui::SliderFloat("Height", &m_Isle.p_Height, 0.0, 100.0);
	ImGui::SliderFloat("Scale", &m_Isle.p_Scale, 0.0, 10.0);
	if (ImGui::Button("Generate new")) m_Isle.GenerateRandom();

	ImGui::Checkbox("1. ", &toggleErode); ImGui::SameLine(0.0, 4.0); ImGui::SliderInt("Erosion", &iterErode, 0, 16);
	ImGui::Checkbox("2. ", &toggleDry); ImGui::SameLine(0.0, 4.0); ImGui::SliderInt("Dry", &iterDry, 0, 16);
	ImGui::Checkbox("3. ", &toggleSlide); ImGui::SameLine(0.0, 4.0); ImGui::SliderInt("Slide", &iterSlide, 0, 16);
	ImGui::Checkbox("4. ", &toggleCoral); ImGui::SameLine(0.0, 4.0); ImGui::SliderInt("Coral", &iterCoral, 0, 16);
	ImGui::Checkbox("5. ", &toggleGrow); ImGui::SameLine(0.0, 4.0); ImGui::SliderInt("Growth", &iterGrow, 0, 16);
	ImGui::SliderInt("REPEATS", &repeats, 0, 32); ImGui::SameLine(0.0, 4.0);
	if (ImGui::Button("Start"))
	{
		for (int i = 0; i < repeats; i++)
		{
			for (int i = 0; i < toggleErode * iterErode; i++) m_Isle.Erode();
			for (int i = 0; i < toggleDry * iterDry; i++) m_Isle.Dry();
			for (int i = 0; i < toggleSlide * iterSlide; i++) m_Isle.Slide();
			for (int i = 0; i < toggleCoral * iterCoral; i++) m_Isle.Coral();
			for (int i = 0; i < toggleGrow * iterGrow; i++) m_Isle.Grow();
		}
	}

	if (ImGui::Button("Resolution / 2")) m_Isle.ResolutionHalve(); 
	ImGui::SameLine(0.0, 4.0); 
	if (ImGui::Button("Resolution x 2")) m_Isle.ResolutionDouble();
	ImGui::SameLine(0.0, 4.0);
	if (ImGui::Button("Blur")) m_Isle.Blur();
	if (ImGui::Button("Export")) m_Isle.Export();
	
	ImGui::End();

}
