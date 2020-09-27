#pragma once

#include "Hazel/Core.h"

namespace Hazel {

	struct FramebufferSpecification
	{
		uint32_t Width, Height;

		//// glBindFramebuffer(0) in sort-of API agnostic Cherno style:
		bool SwapChainTarget = false; // true would mean to render to screen. False means render to framebuffer.
		//// 
	};

	class Framebuffer
	{
	public:
		virtual void Bind() = 0;
		virtual void Unbind(int width, int height) = 0;

		virtual uint32_t GetColorAttachmentRendererID() const = 0;
		virtual uint32_t GetDepthAttachmentRendererID() const = 0;

		virtual const FramebufferSpecification& GetSpecification() const = 0;
		
		static Framebuffer* Create(const FramebufferSpecification& spec);
	};

}