#include "hzpch.h"

#include "Hazel/Entity/VisibleWorld.h"
#include <glad/glad.h>

namespace Hazel
{
	VisibleWorld::VisibleWorld(Library& lib, Camera& cam)
		: m_Library(lib), m_Camera(cam)
	{
	}

	void VisibleWorld::Render()
	{
		glEnable(GL_CLIP_DISTANCE0);
		//// Reflection pass
		m_WaterRendererWrapper.BindReflectionFB();
		RenderCommand::SetClearColor(m_Ocean->GetColorVec4(2));
		RenderCommand::Clear();
		m_Camera.InvertY();
		Renderer::BeginScene(m_Camera, m_Weather.SunPosition);
		//this->RenderEntities();
		if (m_Skybox_active)
			m_Skybox.Render(m_Camera);
		m_Isle->Render(m_Camera);
		Renderer::EndScene();
		m_Camera.InvertY();
		glDisable(GL_CLIP_DISTANCE0);
		glEnable(GL_CLIP_DISTANCE1);
		// Refraction pass
		m_WaterRendererWrapper.BindRefractionFB();
		RenderCommand::Clear();
		Renderer::BeginScene(m_Camera, m_Weather.SunPosition);
		this->RenderEntities();
		m_Isle->Render(m_Camera);
		Renderer::EndScene();
		glDisable(GL_CLIP_DISTANCE1);
		// Render to screen
		m_WaterRendererWrapper.BindScreen();
		RenderCommand::SetClearColor(glm::vec4(1.0, 1.0, 1.0, 1.0));
		RenderCommand::Clear();
		Renderer::BeginScene(m_Camera, m_Weather.SunPosition);
		this->RenderEntities();
		m_Ocean->Render(m_Camera);
		m_Isle->Render(m_Camera);
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