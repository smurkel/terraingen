#include "TerrainTests.h"
#include "Hazel/Events/Event.h"

TerrainTests::TerrainTests()
	: Layer("Test layer"), m_CameraController(), m_VisibleWorld(m_CameraController.GetCamera())
{
	m_VisibleWorld.AddIsle(&m_Isle);
	
	//m_Isle.Sunlight();
	m_VisibleWorld.m_Skybox.SetCubemap(0);
	Hazel::Ref<Hazel::Shader> shader = Hazel::Shader::Create("assets/shaders/SkyBoxShader.glsl");
	m_VisibleWorld.m_Skybox.SetShader(shader);
	m_Ocean.Generate(128, 1.0, glm::vec2(7.0, 7.0), 10, false);
	m_Isle.WATERLEVEL = m_Isle.p_Height;
	m_VisibleWorld.SetOcean(&m_Ocean);
	//m_Ocean.SetMurkiness(2.0);	
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
	RenderingImGuiSettings();
	IsleImGuiSettings();
	SimulationImGuiSettings();
	DebugImGuiSettings();
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
	if (e.GetKeyCode() == HZ_KEY_W)
		m_Isle.WATERLEVEL += 3.0;
	if (e.GetKeyCode() == HZ_KEY_S)
		m_Isle.WATERLEVEL -= 3.0;
	if (e.GetKeyCode() == HZ_KEY_A)
		m_VisibleWorld.ToggleSkyboxActive();
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
	if (e.GetKeyCode() == HZ_KEY_LEFT)
		m_Isle.SwitchDown();
	if (e.GetKeyCode() == HZ_KEY_RIGHT)
		m_Isle.SwitchUp();
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

void TerrainTests::RenderingImGuiSettings()
{
	ImGui::Begin("Render settings");
	ImGui::ColorEdit4("Soiltype: bedrock", &m_Isle.terrainColorR.r);
	ImGui::ColorEdit4("Soiltype: gravel", &m_Isle.terrainColorG.r);
	ImGui::ColorEdit4("Soiltype: sand", &m_Isle.terrainColorB.r);
	ImGui::ColorEdit4("Soiltype: humus", &m_Isle.terrainColorA.r);
	ImGui::ColorEdit4("Plant species 1 (tree)", &m_Isle.plantColorR.r);
	ImGui::ColorEdit4("Plant species 2 (shrub)", &m_Isle.plantColorG.r);
	ImGui::ColorEdit4("Plant species 3 (grass)", &m_Isle.plantColorB.r);
	ImGui::ColorEdit4("Plant species 4 (lichen)", &m_Isle.plantColorA.r);
	float Transparency = m_Ocean.GetMurkiness();
	ImGui::SliderFloat("Water clarity", &Transparency, 0.0, 50.0);
	m_Ocean.SetMurkiness(Transparency);
	float _size = m_Ocean.GetSimulationSize();
	ImGui::SliderFloat("Ocean scale", &_size, 10.0, 5000.0);
	m_Ocean.SetSimulationSize(_size);
	//glm::vec4 cE = m_Ocean.GetColorVec4(0);
	glm::vec4 cA = m_Ocean.GetColorVec4(1);
	glm::vec4 cD = m_Ocean.GetColorVec4(2);
	//glm::vec4 cS = m_Ocean.GetColorVec4(3);
	//ImGui::ColorEdit4("Emissive colour", &cE.r);
	ImGui::ColorEdit4("Ambient water colour", &cA.r);
	ImGui::ColorEdit4("Diffuse water colour", &cD.r);
	//ImGui::ColorEdit4("Specular colour", &cS.r);
	//m_Ocean.SetColorVec4(0, cE);
	m_Ocean.SetColorVec4(1, cA);
	m_Ocean.SetColorVec4(2, cD);
	//m_Ocean.SetColorVec4(3, cS);
	ImGui::End();
}

void TerrainTests::DebugImGuiSettings()
{
	ImGui::Begin("Debug");
	ImGui::Image((void*)m_Ocean._GetBFTextureID(), ImVec2(256, 256));
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

void TerrainTests::SimulationImGuiSettings()
{
	ImGui::Begin("Simulation settings");
	ImGui::Text("Erosion settings");
	ImGui::SliderFloat("Inertia", &m_Isle.INERTIA, 0.0, 1.0);
	ImGui::SliderFloat("Erosion rate", &m_Isle.EROSION_RATE, 0.0, 1.0);
	ImGui::SliderFloat("Deposition rate", &m_Isle.DEPOSITION_RATE, 0.0, 1.0);
	ImGui::SliderFloat("Deposition rate underwater", &m_Isle.DEPOSIT_UNDERWATER, 0.0, 1.0);
	ImGui::SliderFloat("Gravity", &m_Isle.GRAVITY, 0.0, 20.0);
	ImGui::SliderFloat("Evaporation", &m_Isle.EVAPORATION_COEFFICIENT, 0.0, 1.0);
	ImGui::SliderFloat("Rock -> Pebble degradation", &m_Isle.SOIL_DEGRADATION_RG, 0.0, 1.0);
	ImGui::SliderFloat("Rock  ->  Sand degradation", &m_Isle.SOIL_DEGRADATION_RB, 0.0, (1.0 - m_Isle.SOIL_DEGRADATION_RG));
	ImGui::SliderFloat("Pebble -> Sand degradation", &m_Isle.SOIL_DEGRADATION_GB, 0.0, 1.0);
	ImGui::InputFloat4("Type hardness", &m_Isle.SOIL_HEALTH.r, 2);
	ImGui::InputFloat4("Type sedimentation rate", &m_Isle.SEDIMENTATION_RATE.r, 2);
	ImGui::InputFloat4("Erosion max layer fraction", &m_Isle.SOIL_ERODE_MAX_LAYER_FRAC.r, 2);
	if (ImGui::Button("Erosion iteration")) m_Isle.Erode();
	ImGui::Text("Landslide settings");
	ImGui::InputFloat4("Friction angles", &m_Isle.GRAV_FRICTION_ANGLE.r, 1);
	ImGui::SliderFloat("Underwater friction angle multiplier", &m_Isle.GRAV_FRICTION_ANGLE_UNDERWATER_MULTIPLIER, 0.0, 3.0);
	ImGui::SliderFloat("Min layer fraction", &m_Isle.GRAV_FRAC_MIN, 0.0, 1.0);
	ImGui::SliderFloat("Max layer fraction", &m_Isle.GRAV_FRAC_MAX, m_Isle.GRAV_FRAC_MIN, 1.0);
	if (ImGui::Button("Landslide iteration")) m_Isle.Slide();
	ImGui::Text("Vegatation settings");
	ImGui::InputFloat4("Type max population", &m_Isle.PL_MAX_POP.r, 2);
	ImGui::InputFloat("Max total population", &m_Isle.PL_MAX_POP_TOTAL, 0.1, 0.5);
	ImGui::InputFloat4("Type max age", &m_Isle.PL_MAX_AGE.r, 2);
	ImGui::InputFloat4("Type growth rate", &m_Isle.PL_TYPE_GROWTH_RATE.r, 2);
	ImGui::InputFloat4("Type gradient ideal", &m_Isle.PL_GRADIENT_IDEAL.r, 2);
	ImGui::InputFloat4("Type gradient range", &m_Isle.PL_GRADIENT_RANGE.r, 2);
	ImGui::InputFloat4("Type humidity ideal", &m_Isle.PL_HUMIDITY_IDEAL.r, 2);
	ImGui::InputFloat4("Type humidity range", &m_Isle.PL_HUMIDITY_RANGE.r, 2);
	ImGui::InputFloat4("Type humus yield", &m_Isle.PL_HUMUS_PROD.r, 2);
	ImGui::InputFloat4("Type fertility minimum", &m_Isle.PL_HUMUS_MIN.r, 2);
	ImGui::InputFloat4("Type fertility bonus", &m_Isle.PL_HUMUS_GROWTH_SPONSOR.r, 2);
	ImGui::SliderFloat("Base death rate", &m_Isle.PL_DEATH_RATE, 0.0, 1.0);
	ImGui::SliderFloat("Base growth rate", &m_Isle.PL_GROWTH_RATE, 0.0, 1.0);
	ImGui::SliderFloat("Base aging rate", &m_Isle.PL_AGE_RATE, 0.0, 1.0);
	ImGui::SliderFloat("Overgrowth attrition", &m_Isle.PL_OVERGROWTH_ATTRITION, 0.0, 1.0);
	ImGui::SliderFloat("Humus decay rate", &m_Isle.HUMUS_DECAY_RATE, 0.0, 1.0);
	ImGui::InputFloat3("Underwater humus petrification product", &m_Isle.HUMUS_UNDERWATER_PETRIFY_SOIL.r, 2);
	if (ImGui::Button("Vegetation iteration")) m_Isle.Grow();
	ImGui::Text("Vegetation - erosion interactions");
	ImGui::InputFloat4("Erosion damping", &m_Isle.PL_SOIL_PROTECT_FAC.r, 2);
	ImGui::InputFloat4("Deposition increase", &m_Isle.PL_SOIL_DEPOSIT_FAC.r, 2);
	ImGui::InputFloat4("Moisture absorption rate", &m_Isle.PL_INITIAL_MOISTURE_ABSORB.r, 2);
	ImGui::Text("Ocean settings");
	ImGui::SliderFloat("Water level", &m_Isle.WATERLEVEL, -m_Isle.p_Height, 3.0 * m_Isle.p_Height);
	ImGui::SliderFloat("Coral gradient ideal", &m_Isle.CORAL_GRADIENT_IDEAL, 0.0, 90.0);
	ImGui::SliderFloat("Coral gradient range", &m_Isle.CORAL_GRADIENT_RANGE, 0.0, 90.0);
	ImGui::SliderFloat("Coral depth ideal", &m_Isle.CORAL_DEPTH_IDEAL, 0.0, 20.0);
	ImGui::SliderFloat("Coral depth max", &m_Isle.CORAL_DEPTH_MAX, m_Isle.CORAL_DEPTH_IDEAL, 100.0);
	ImGui::SliderFloat("Coral depth min", &m_Isle.CORAL_DEPTH_MIN, 0.0, m_Isle.CORAL_DEPTH_IDEAL);
	ImGui::SliderFloat("Coral base growth", &m_Isle.CORAL_BASE_GROWTH, 0.0, 5.0);
	ImGui::SliderFloat("Coral max growth rate", &m_Isle.CORAL_GROWTH_MAX, 0.0, 1.0);
	ImGui::InputFloat4("Coral soiltype", &m_Isle.CORAL_SOILTYPE.r, 2);
	if (ImGui::Button("Coral iteration")) m_Isle.Coral();
	ImGui::End();
	

}
