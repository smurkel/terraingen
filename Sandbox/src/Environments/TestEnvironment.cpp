#include "TestEnvironment.h"
#include "Hazel/Events/Event.h"

TestEnvironment::TestEnvironment()
	: Layer("Test layer"), m_CameraController(), m_VisibleWorld(m_Library, m_CameraController.GetCamera())
{
	m_Ocean.Generate(256, 100, 0.1, glm::vec2(5.0, 2.0), 3);
	//m_WPE.SetOcean(&m_Ocean);
	m_VisibleWorld.SetOcean(&m_Ocean);
	m_VisibleWorld.SetTerrain(&m_Terrain);
	//m_Boat.SetShader(m_Library.GetShader("assets/shaders/DefaultPolyShader.glsl"));
	//m_Boat.SetModel(m_Library.GetModel("assets/models/RaftValid.obj", "assets/textures/Palettes.png"));
	//m_VisibleWorld.AddEntity(&m_Boat);

	m_VisibleWorld.SetCamera(m_CameraController.GetCamera());
}

void TestEnvironment::OnUpdate(Hazel::Timestep ts)
{
	
	m_Ocean.Update(ts);
	//m_Boat.OnUpdate(ts, m_WPE);
	////m_CameraController.SetFocus(m_Boat.GetPosition());
	//
	m_CameraController.OnUpdate(ts);

	//Hazel::RenderCommand::SetClearColor(glm::vec4(0.2, 0.5, 0.7, 1.0));
	//Hazel::RenderCommand::Clear();
	//Hazel::Renderer::BeginScene(m_CameraController.GetCamera(), glm::vec3(1.0, 1.0, 1.0));
	//m_Terrain.Render(m_CameraController.GetCamera());
	
	//m_Ocean.Render(m_CameraController.GetCamera());
	//Hazel::Renderer::EndScene();
	HZ_CORE_INFO("fps: {0}", (1.0 / ts.GetSeconds()));
	m_VisibleWorld.Render();
	c_Emissive = m_Ocean.GetColorVec4(0);
	c_Ambient = m_Ocean.GetColorVec4(1);
	c_Diffuse = m_Ocean.GetColorVec4(2);
	c_Specular = m_Ocean.GetColorVec4(3);
}

void TestEnvironment::OnImGuiRender()
{
	ImGui::Begin("Terrain generator");

	ImGui::Checkbox("Lock seed", &keepSeed);
	ImGui::SliderInt("Resolution", &N, 16, 2048);
	ImGui::SliderInt("Octaves", &octaves, 1, 32);
	ImGui::SliderFloat("Lacunarity", &lacunarity, 0.2, 4.0);
	ImGui::SliderFloat("Persistence", &persistence, 0.05, 1.0);
	ImGui::SliderFloat("Scale", &scale, 0.5, 20);
	ImGui::SliderFloat("Height", &height, -10.0, 10.0);
	ImGui::SliderFloat("Area", &gridsize, 1.0, 200.0);
	bool newMapRequested = ImGui::Button("Generate");
	if (newMapRequested)
	{
		if (!keepSeed)
		{ 
			m_Terrain.DeleteCSTextures();
			m_Terrain.NewSeed();
			seed = m_Terrain.GetSeed();
			HZ_CORE_INFO("Current seed: {0}", seed);
		}
		
		m_Terrain = Hazel::Terrain(seed, N, gridsize, scale, height, octaves, persistence, lacunarity);
		m_VisibleWorld.SetTerrain(&m_Terrain);
	}
	bool exportMap = ImGui::Button("Export");
	if (exportMap)
	{
		m_Terrain.Export();
	}
	ImGui::End();

	ImGui::Begin("Erosion simulation");
	ImGui::SliderInt("Iterations", &ITERATIONS, 1, 32);
	ImGui::SliderFloat("VELOCITY", &VELOCITY, 0, 2);
	ImGui::SliderFloat("VOLUME", &VOLUME, 0, 2);
	ImGui::SliderFloat("INERTIA", &INERTIA, 0, 1);
	ImGui::SliderFloat("EROSION", &EROSION, 0, 1);
	ImGui::SliderFloat("DEPOSITION", &DEPOSITION, 0, 1);
	ImGui::SliderFloat("EVAPORATION", &EVAPORATION, 0, 1);
	ImGui::SliderFloat("WATERLEVEL", &WATERLEVEL, -10.0, 10.0);
	ImGui::SliderFloat("GRAVITY", &GRAVITY, -10, 50);
	ImGui::SliderFloat("CAPACITY", &CAPACITY, 0, 8);
	ImGui::SliderInt("MAXSTEPS", &MAXSTEPS, 1, 128);
	ImGui::SliderInt("MINSLOPE_EXPONENT", &MINSLOPE_EXP, -10, -1);
	MINSLOPE = std::pow(1, MINSLOPE_EXP);
	ImGui::Checkbox("Invert erosion", &INVERTEROSION);
	bool erode = ImGui::Button("Erode");
	if (erode)
	{
		m_Terrain.Erode(ITERATIONS, VELOCITY, VOLUME, INERTIA, CAPACITY, EROSION, GRAVITY, EVAPORATION, DEPOSITION, MINSLOPE, MAXSTEPS, INVERTEROSION);
	}
	bool blur = ImGui::Button("Blur");
	if (blur)
	{
		m_Terrain.Blur();
	}
	ImGui::End();
	ImGui::Begin("Shading");
	float _temp = m_Terrain.GetHeightOffset();
	ImGui::SliderFloat("Height offset", &_temp, -10.0, 10.0);
	m_Terrain.SetHeightOffset(_temp);
	_temp = m_Terrain.GetTextureOffset();
	ImGui::SliderFloat("Texture offset", &_temp, -10.0, 10.0);
	m_Terrain.SetTextureOffset(_temp);
	_temp = m_Terrain.GetErosionWeight();
	ImGui::SliderFloat("Erosion weight", &_temp, -1.0, 1.0);
	m_Terrain.SetErosionWeight(_temp);

	_temp = m_Terrain.GetHeightScalingFactor();
	ImGui::SliderFloat("Height scaling", &_temp, -10.0, 10.0);
	m_Terrain.SetHeightScalingFactor(_temp);
	ImGui::SliderFloat("Aridity", &ARIDITY, 0.0, 2.0);
	ImGui::SliderFloat("Growth steepness", &GROWTHSTEEPNESS, 0.0, 10.0);
	m_Terrain.SetLushOffset(ARIDITY);
	m_Terrain.SetLushScale(GROWTHSTEEPNESS);
	bool updateTexture = ImGui::Button("Update Texture");
	if (updateTexture)
	{
		m_Terrain.UpdateTexture();
	}
	_temp = m_Ocean.GetMurkiness();
	ImGui::SliderFloat("Murkiness", &_temp, 0.0, 25.0);
	m_Ocean.SetMurkiness(_temp);
	ImGui::ColorEdit3("Emissive", &c_Emissive[0]);
	ImGui::ColorEdit3("Ambient", &c_Ambient[0]);
	ImGui::ColorEdit3("Diffuse", &c_Diffuse[0]);
	ImGui::ColorEdit3("Specular", &c_Specular[0]);
	m_Ocean.SetColorVec4(0, c_Emissive);
	m_Ocean.SetColorVec4(1, c_Ambient);
	m_Ocean.SetColorVec4(2, c_Diffuse);
	m_Ocean.SetColorVec4(3, c_Specular);
	ImGui::Image((void*)m_Terrain.GetErosionMapID(), ImVec2{ 256.0f, 256.0f });
	ImGui::Image((void*)m_Terrain.GetBlurMapID(), ImVec2{ 256.0f, 256.0f });
	ImGui::End();
	

	
}

void TestEnvironment::OnEvent(Hazel::Event& e)
{
	m_CameraController.OnEvent(e);
	Hazel::EventDispatcher dispatcher(e);
	dispatcher.Dispatch<Hazel::KeyPressedEvent>(HZ_BIND_EVENT_FN(TestEnvironment::OnKeyPressed));
}

bool TestEnvironment::OnKeyPressed(Hazel::KeyPressedEvent& e)
{
	glm::vec2 wind = m_Ocean.GetWind();
	if (e.GetKeyCode() == HZ_KEY_I) wind.x = wind.x + 1;
	if (e.GetKeyCode() == HZ_KEY_K) wind.x = wind.x - 1;
	if (e.GetKeyCode() == HZ_KEY_J) wind.y = wind.y - 1;
	if (e.GetKeyCode() == HZ_KEY_L) wind.y = wind.y + 1;
	m_Ocean.SetWind(wind);
	
	//HZ_CORE_INFO("Wind: ({0}, {1})", m_Ocean.GetWind().x, m_Ocean.GetWind().y);

	float amp = m_Ocean.GetAmplitude();
	if (e.GetKeyCode() == HZ_KEY_Q) amp += 0.025;
	if (e.GetKeyCode() == HZ_KEY_E) amp -= 0.025;
	if (e.GetKeyCode() == HZ_KEY_SPACE) m_Ocean.InvDynamic();

	m_Ocean.SetAmplitude(amp);
	//HZ_CORE_INFO("Amplitude = {0}", m_Ocean.GetAmplitude());

	m_Ocean.UpdateSpectrum();
	return true;
}