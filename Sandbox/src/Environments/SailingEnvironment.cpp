
#include "SailingEnvironment.h"
#include "Hazel/Events/Event.h"

SailingEnvironment::SailingEnvironment()
	: Layer("Test layer"), m_CameraController(), m_VisibleWorld(m_Library, m_CameraController.GetCamera())
{
	m_Ocean.Generate(256, 210, 0.1, glm::vec2(5.0, 2.0), 3);
	m_VisibleWorld.SetOcean(&m_Ocean);
	m_Island.SetShader(m_Library.GetShader("assets/shaders/DefaultPolyShader.glsl"));
	m_Island.SetModel(m_Library.GetModel("assets/Islands/Island5.obj", "assets/Islands/Island5Texture.png"));
	m_VisibleWorld.AddEntity(&m_Island);

	m_VisibleWorld.SetCamera(m_CameraController.GetCamera());
}

void SailingEnvironment::OnUpdate(Hazel::Timestep ts)
{

	m_Ocean.Update(ts);
	m_CameraController.OnUpdate(ts);
	

	m_VisibleWorld.Render();

	c_Emissive = m_Ocean.GetColorVec4(0);
	c_Ambient = m_Ocean.GetColorVec4(1);
	c_Diffuse = m_Ocean.GetColorVec4(2);
	c_Specular = m_Ocean.GetColorVec4(3);
	HZ_CORE_INFO("framerate: {0}",1.0/ts.GetSeconds());
}

void SailingEnvironment::OnImGuiRender()
{
	ImGui::Begin("Shader settings");
	float _temp = m_Ocean.GetMurkiness();
	ImGui::SliderFloat("Murkiness", &_temp, 0.0, 25.0);
	m_Ocean.SetMurkiness(_temp);
	glm::vec3 pos = m_Island.GetPosition();
	float x = pos.x;
	float y = pos.y;
	float z = pos.z;
	ImGui::SliderFloat("X", &x, -100.0, 100.0);
	ImGui::SliderFloat("Y", &y, -100.0, 100.0);
	ImGui::SliderFloat("Z", &z, -100.0, 100.0);
	glm::vec3 newpos = glm::vec3(x, y, z);
	m_Island.SetPosition(newpos);
	ImGui::End();
}

void SailingEnvironment::OnEvent(Hazel::Event& e)
{
	m_CameraController.OnEvent(e);
	Hazel::EventDispatcher dispatcher(e);
	dispatcher.Dispatch<Hazel::KeyPressedEvent>(HZ_BIND_EVENT_FN(SailingEnvironment::OnKeyPressed));
}

bool SailingEnvironment::OnKeyPressed(Hazel::KeyPressedEvent& e)
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