#include "hzpch.h"
#include "Framebuffer.h"

#include "Platform/OpenGL/OpenGlFramebuffer.h"
#include "Hazel/Renderer/Renderer.h"


namespace Hazel {



	Framebuffer* Framebuffer::Create(const FramebufferSpecification& spec)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:	HZ_CORE_ASSERT(false, "RendererAP::None is currently not supported!"); return nullptr;
			case RendererAPI::API::OpenGL:	return new OpenGLFramebuffer(spec);
		}

		HZ_CORE_ASSERT(false, "No valid RendererAPI selected");
		return nullptr;
	}

}