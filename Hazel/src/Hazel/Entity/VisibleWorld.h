#pragma once

#include "Hazel/Ocean/Ocean.h"
//#include "Hazel/Experimental/Terrain.h"
#include "Hazel/Experimental/Isle.h"
#include "Hazel/Experimental/Skybox.h"
#include "Hazel/Renderer/Framebuffer.h"
#include "Hazel/Renderer/Renderer.h"
#include "Hazel/ECS/Systems/RenderSystem.h"

namespace Hazel
{
	struct Weather
	{
		glm::vec3 SunPosition = glm::vec3{ 0.0, 10000.0, 15000.0 };
	};

	class WaterRendererWrapper
	{
	public:
		WaterRendererWrapper();

		void SetOcean(Ocean* ocean) { m_Ocean = ocean; }

		void BindReflectionFB() { fb_Reflection->Bind(); }
		void BindRefractionFB() { fb_Refraction->Bind(); }
		void BindScreen() { fb_Refraction->Unbind(); }
		uint32_t GetReflectionTextureID() { return fb_Reflection->GetColorAttachmentRendererID(); }
		uint32_t GetReflectionDepthTextureID() { return fb_Reflection->GetDepthAttachmentRendererID(); }
		uint32_t GetRefractionTextureID() { return fb_Refraction->GetColorAttachmentRendererID(); }
		uint32_t GetRefractionDepthTextureID() { return fb_Refraction->GetDepthAttachmentRendererID(); }

		float CLIPPING_MARGIN_REFLECTION = -2.0;
		float CLIPPING_MARGIN_REFRACTION = 2.0;
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
	public:
		VisibleWorld(Camera& cam);
		void AddIsle(Isle* isle) { m_Isle = isle; m_Isle_active = true;	}
		void SetOcean(Ocean* ocean) {
			m_Ocean = ocean;
			m_WaterRendererWrapper.SetOcean(ocean);
			m_Ocean->SetReflectionTextureID(m_WaterRendererWrapper.GetReflectionTextureID());
			m_Ocean->SetRefractionTextureID(m_WaterRendererWrapper.GetRefractionTextureID());
			m_Ocean->SetRefractionDepthTextureID(m_WaterRendererWrapper.GetRefractionDepthTextureID());
			m_Ocean->SetSunPos(m_Weather.SunPosition);
		}
		void ToggleSkyboxActive() { m_Skybox_active = !m_Skybox_active; }
		
		void SetCamera(Camera& camera) { m_Camera = camera; }
		void SetRenderSystem(Ref<RenderSystem> rendersystem) { m_RenderSystem = rendersystem; m_RenderSystem_active = true; }
		void Render();

		uint32_t GetReflectionImage() { return m_WaterRendererWrapper.GetReflectionTextureID(); }
		uint32_t GetRefractionImage() { return m_WaterRendererWrapper.GetRefractionTextureID(); }
		uint32_t GetRefractionDepthImage() { return m_WaterRendererWrapper.GetRefractionDepthTextureID(); }
		uint32_t GetReflectionDepthImage() { return m_WaterRendererWrapper.GetReflectionDepthTextureID(); }
		
	public:
		Camera& m_Camera;		
		// other
		Weather m_Weather;
		Hazel::Ocean* m_Ocean = nullptr;

		// skybox
		Hazel::Skybox m_Skybox;
		bool m_Skybox_active = true;
		// isle
		Hazel::Isle* m_Isle = nullptr;
		bool m_Isle_active = false;
		// entity rendering
		Hazel::Ref<Hazel::RenderSystem> m_RenderSystem;
		bool m_RenderSystem_active = false;
	private:
		WaterRendererWrapper m_WaterRendererWrapper = WaterRendererWrapper();
	};

	
}