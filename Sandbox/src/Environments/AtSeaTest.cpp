#include "AtSeaTest.h"
#include "Hazel/Events/Event.h"
#include "Hazel/Renderer/Shader.h"
#include "Hazel/Renderer/Texture.h"
#include "Hazel/Renderer/Model.h"
#include "Hazel/ECS/Components/RenderComponents.h"
#include "Hazel/ECS/Components/Transform.h"
#include "Hazel/ECS/Components/RigidBody.h"
#include "Hazel/ECS/Components/ProbeArrayComponent.h"
#include "Hazel/ECS/Components/BoatParams.h"

AtSeaTest::AtSeaTest()
	: Layer("Test layer"), m_CameraController(), m_VisibleWorld(m_CameraController.GetCamera()), m_BoatController()
{
	////////////////
	// FACTORY STUFF
	////////////////
	// Resource Manager
	m_ResourceManager->RegisterResource<Hazel::Shader>();
	m_ResourceManager->RegisterResource<Hazel::Model>();
	m_ResourceManager->RegisterResource<Hazel::Texture2D>();
	m_ResourceManager->RegisterResource<Hazel::ProbeArray>();
	// ECS Setup
	m_Coordinator.Init();
		// Register components
	m_Coordinator.RegisterComponent<Hazel::TransformComponent>();
	m_Coordinator.RegisterComponent<Hazel::ShaderComponent>();
	m_Coordinator.RegisterComponent<Hazel::ModelComponent>();
	m_Coordinator.RegisterComponent<Hazel::TextureComponent>();
	//m_Coordinator.RegisterComponent<Hazel::RigidBody>();
	//m_Coordinator.RegisterComponent<Hazel::ProbeArrayComponent>();
	m_Coordinator.RegisterComponent<Hazel::BoatParameters>();
		// Register systems
			// Render system
	m_RenderSystem = m_Coordinator.RegisterSystem<Hazel::RenderSystem>();
	{
		m_RenderSystem->Init(&m_Coordinator);
		Hazel::Signature signature;
		signature.set(m_Coordinator.GetComponentType<Hazel::TransformComponent>());
		signature.set(m_Coordinator.GetComponentType<Hazel::ShaderComponent>());
		signature.set(m_Coordinator.GetComponentType<Hazel::ModelComponent>());
		signature.set(m_Coordinator.GetComponentType<Hazel::TextureComponent>());
		m_Coordinator.SetSystemSignature<Hazel::RenderSystem>(signature);
	}
			// Boat system
	m_BoatSystem = m_Coordinator.RegisterSystem<Hazel::BoatSystem>();
	{
		m_BoatSystem->Init(&m_Coordinator);
		Hazel::Signature signature;
		signature.set(m_Coordinator.GetComponentType<Hazel::TransformComponent>());
		signature.set(m_Coordinator.GetComponentType<Hazel::BoatParameters>());
		m_Coordinator.SetSystemSignature<Hazel::BoatSystem>(signature);
	}
			// Physics system
	//m_PhysicsSystem = m_Coordinator.RegisterSystem<Hazel::PhysicsSystem>();
	//{
	//	m_PhysicsSystem->Init(&m_Coordinator, m_Ocean);
	//	Hazel::Signature signature;
	//	signature.set(m_Coordinator.GetComponentType<Hazel::TransformComponent>());
	//	signature.set(m_Coordinator.GetComponentType<Hazel::RigidBody>());
	//	signature.set(m_Coordinator.GetComponentType<Hazel::ProbeArrayComponent>());
	//	m_Coordinator.SetSystemSignature<Hazel::PhysicsSystem>(signature);
	//}

	// Set up an entity
	Hazel::Entity Boat = m_Coordinator.CreateEntity();
	{
		// add transform
		Hazel::TransformComponent T;
		T.matrix = glm::mat4(1.0);
		m_Coordinator.AddComponent<Hazel::TransformComponent>(Boat, T);
		// add shader
		Hazel::ShaderComponent shader;
		shader.shader = m_ResourceManager->GetResource<Hazel::Shader>("assets/shaders/DefaultPolyShader.glsl");
		m_Coordinator.AddComponent<Hazel::ShaderComponent>(Boat, shader);
		// add model
		Hazel::ModelComponent model;
		model.model = m_ResourceManager->GetResource<Hazel::Model>("assets/models/Sloop.obj");
		m_Coordinator.AddComponent<Hazel::ModelComponent>(Boat, model);
		// add texture
		Hazel::TextureComponent texture;
		texture.texture = m_ResourceManager->GetResource<Hazel::Texture2D>("assets/textures/Palettes.png");
		m_Coordinator.AddComponent<Hazel::TextureComponent>(Boat, texture);
		// add boat params
		Hazel::BoatParameters boatparams;
		m_Coordinator.AddComponent<Hazel::BoatParameters>(Boat, boatparams);
		// add probe array
		//Hazel::ProbeArrayComponent probeArray;
		//probeArray.probeArray = m_ResourceManager->GetResource<Hazel::ProbeArray>("assets/models/PyramidProbes.obj");
		//m_Coordinator.AddComponent<Hazel::ProbeArrayComponent>(Boat, probeArray);
		//// add rigid body
		//Hazel::RigidBody rigidBody = Hazel::RigidBody::RigidBody(probeArray.probeArray->GetMass(), probeArray.probeArray->GetMomentOfInertia());
		//m_Coordinator.AddComponent<Hazel::RigidBody>(Boat, rigidBody);
	}	
	////////////////////
	// END FACTORY STUFF
	////////////////////

	/// SET UP BOAT CONTROLLER ///
	m_BoatController.Init(&m_Coordinator);
	m_BoatController.SetCurrentBoat(Boat);
	m_BoatController.SetActive();
	
	/// SET UP VISIBLE WORLD WRAPPER ///
	m_VisibleWorld.SetOcean(&m_Ocean);
	m_VisibleWorld.SetRenderSystem(m_RenderSystem);
	m_VisibleWorld.m_Skybox.SetCubemap(0);
	m_VisibleWorld.m_Skybox.SetShader(m_ResourceManager->GetResource<Hazel::Shader>("assets/shaders/SkyBoxShader.glsl"));
	m_Ocean.Generate(128, 1.0, glm::vec2(10.0, 2.0), 2, false);
}

void AtSeaTest::OnUpdate(Hazel::Timestep ts)
{
	m_Ocean.Update(ts);
	m_CameraController.OnUpdate(ts);
	Hazel::Entity currentFocusEntity = m_BoatController.GetCurrentBoat();
	Hazel::BoatParameters bp = m_Coordinator.GetComponent<Hazel::BoatParameters>(currentFocusEntity);
	glm::vec3 focus = glm::vec3(bp.position.x, 0.0f, bp.position.y);
	m_CameraController.SetFocus(focus);
	//m_PhysicsSystem->OnUpdate(ts);
	m_BoatController.OnUpdate(ts, m_CameraController.GetCamera());
	m_BoatSystem->OnUpdate(ts, glm::vec2(10.0, 2.0));
 	m_VisibleWorld.Render();
}

void AtSeaTest::OnImGuiRender()
{
	ImGui::Begin("ImGui");
	if (ImGui::Button("Add entity"))
	{
		for (int i = 0; i < 1; i++)
		{
			Hazel::ShaderComponent shader;
			shader.shader = m_ResourceManager->GetResource<Hazel::Shader>("assets/shaders/DefaultPolyShader.glsl");
			Hazel::ModelComponent model;
			model.model = m_ResourceManager->GetResource<Hazel::Model>("assets/models/Sloop.obj");
			Hazel::TextureComponent texture;
			texture.texture = m_ResourceManager->GetResource<Hazel::Texture2D>("assets/textures/Palettes.png");
			Hazel::Entity E = m_Coordinator.CreateEntity();
			Hazel::TransformComponent Tr;
			glm::vec3 Pos{ std::rand() % 100 - 50, std::rand() % 10 - 5, std::rand() % 100 - 50 };
			Tr.matrix = glm::translate(glm::mat4(1.0), glm::vec3(Pos));
			m_Coordinator.AddComponent<Hazel::TransformComponent>(E, Tr);
			m_Coordinator.AddComponent<Hazel::ShaderComponent>(E, shader);
			m_Coordinator.AddComponent<Hazel::ModelComponent>(E, model);
			m_Coordinator.AddComponent<Hazel::TextureComponent>(E, texture);
			//// add probe array
			//Hazel::ProbeArrayComponent probeArray;
			//probeArray.probeArray = m_ResourceManager->GetResource<Hazel::ProbeArray>("assets/models/PyramidProbes.obj");
			//m_Coordinator.AddComponent<Hazel::ProbeArrayComponent>(E, probeArray);
			//// add rigid body
			//Hazel::RigidBody rigidBody = Hazel::RigidBody::RigidBody(probeArray.probeArray->GetMass(), probeArray.probeArray->GetMomentOfInertia());
			//rigidBody.x = Pos;
			//m_Coordinator.AddComponent<Hazel::RigidBody>(E, rigidBody);
		}
	}
	ImGui::End();
}

void AtSeaTest::OnEvent(Hazel::Event& e)
{
	m_CameraController.OnEvent(e);
	Hazel::EventDispatcher dispatcher(e);
	dispatcher.Dispatch<Hazel::KeyPressedEvent>(HZ_BIND_EVENT_FN(AtSeaTest::OnKeyPressed));
}

bool AtSeaTest::OnKeyPressed(Hazel::KeyPressedEvent& e)
{
	return true;
}
