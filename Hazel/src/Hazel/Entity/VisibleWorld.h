#pragma once

#include "Hazel/Entity/Entity.h"
#include "Hazel/Ocean/Ocean.h"
#include "Hazel/Experimental/Terrain.h"
#include "Hazel/Renderer/Framebuffer.h"

namespace Hazel
{
	struct Weather
	{
		glm::vec3 SunPosition = glm::vec3{ 20000.0, 10000.0, 0.0 };
	};

	class WaterRendererWrapper
	{
	public:
		WaterRendererWrapper();

		void SetOcean(Ocean* ocean) { m_Ocean = ocean; }

		void BindReflectionFB() { fb_Reflection->Bind(); }
		void BindRefractionFB() { fb_Refraction->Bind(); }
		void BindScreen() { fb_Refraction->Unbind(SCREEN_WIDTH, SCREEN_HEIGHT); }
		uint32_t GetReflectionTextureID() { return fb_Reflection->GetColorAttachmentRendererID(); }
		uint32_t GetRefractionTextureID() { return fb_Refraction->GetColorAttachmentRendererID(); }
		uint32_t GetRefractionDepthTextureID() { return fb_Refraction->GetDepthAttachmentRendererID(); }
	private:
		Ocean* m_Ocean;
		Framebuffer* fb_Reflection;
		Framebuffer* fb_Refraction;
		FramebufferSpecification fb_Reflection_spec;
		FramebufferSpecification fb_Refraction_spec;

		int REFLECTION_WIDTH = 1200;
		int REFLECTION_HEIGHT = 780;
		int REFRACTION_WIDTH = 1200;
		int REFRACTION_HEIGHT = 780;
		int SCREEN_WIDTH = 1200;
		int SCREEN_HEIGHT = 780;
	};
	
	class VisibleWorld
	{
		// Container class for all renderables.
		// Has a list of all rendered objects,
		// knows the Ocean object.
	public:
		VisibleWorld(Library& lib, Camera& cam);
		void AddEntity(Entity* entity) { m_EntityStack.Add(entity); }
		void SetOcean(Ocean* ocean) {
			m_Ocean = ocean;
			m_WaterRendererWrapper.SetOcean(ocean);
			m_Ocean->SetReflectionTextureID(m_WaterRendererWrapper.GetReflectionTextureID());
			m_Ocean->SetRefractionTextureID(m_WaterRendererWrapper.GetRefractionTextureID());
			m_Ocean->SetRefractionDepthTextureID(m_WaterRendererWrapper.GetRefractionDepthTextureID());
			m_Ocean->SetSunPos(m_Weather.SunPosition);
		}
		void SetTerrain(Terrain* terrain) { m_Terrain = terrain; m_Terrain->SetSunPosition(m_Weather.SunPosition); }
		void SetCamera(Camera& camera) { m_Camera = camera; }
		void Render();

		uint32_t GetReflectionImage() { return m_WaterRendererWrapper.GetReflectionTextureID(); }
		uint32_t GetRefractionImage() { return m_WaterRendererWrapper.GetRefractionTextureID(); }
		uint32_t GetDepthImage() { return m_WaterRendererWrapper.GetRefractionDepthTextureID(); }
		
	public:
		// on init:
		Library& m_Library;
		Camera& m_Camera;		
		// other
		Weather m_Weather;
		Hazel::Ocean* m_Ocean = 0;
		Hazel::Terrain* m_Terrain = 0;
		EntityStack m_EntityStack;
	private:
		void RenderEntities();
		WaterRendererWrapper m_WaterRendererWrapper = WaterRendererWrapper();
	};

	
}