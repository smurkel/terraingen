#include "hzpch.h"

#include "Hazel/Entity/VisibleWorld.h"


namespace Hazel
{
	VisibleWorld::VisibleWorld(Library& lib, Camera& cam)
		: m_Library(lib), m_Camera(cam)
	{
	}

	void VisibleWorld::Render()
	{
		//// Reflection pass
		//m_WaterRendererWrapper.BindReflectionFB();
		//RenderCommand::SetClearColor(glm::vec4(0.2, 0.5, 0.7, 1.0));
		//RenderCommand::Clear();
		//m_Camera.InvertY();
		//Renderer::BeginScene(m_Camera, m_Weather.SunPosition);
		//this->RenderEntities();
		//m_Terrain->Render(m_Camera);
		//Renderer::EndScene();
		//m_Camera.InvertY();

		// Refraction pass
		m_WaterRendererWrapper.BindRefractionFB();
		RenderCommand::Clear();
		Renderer::BeginScene(m_Camera, m_Weather.SunPosition);
		//HZ_CORE_INFO("Reflection pass camera position: {0}", m_Camera.GetPositionXYZ());
		this->RenderEntities();
		m_Terrain->Render(m_Camera);
		Renderer::EndScene();

		// Render to screen
		m_WaterRendererWrapper.BindScreen();
		RenderCommand::SetClearColor(glm::vec4(1.0, 1.0, 1.0, 1.0));
		RenderCommand::Clear();
		Renderer::BeginScene(m_Camera, m_Weather.SunPosition);
		this->RenderEntities();
		m_Ocean->Render(m_Camera);
		m_Terrain->Render(m_Camera);
		Renderer::EndScene();
	}

	void VisibleWorld::RenderEntities()
	{
		for (Entity* entity : m_EntityStack)
		{
			entity->Render();
		}
	}

	WaterRendererWrapper::WaterRendererWrapper()
	{
		fb_Reflection_spec.Width = REFLECTION_WIDTH;
		fb_Reflection_spec.Height = REFLECTION_HEIGHT;
		fb_Refraction_spec.Width = REFRACTION_WIDTH;
		fb_Refraction_spec.Height = REFRACTION_HEIGHT;
		fb_Reflection = Framebuffer::Create(fb_Reflection_spec);
		fb_Refraction = Framebuffer::Create(fb_Refraction_spec);
	}

}