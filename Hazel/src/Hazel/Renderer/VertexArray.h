#pragma once

#include "Hazel/Renderer/Buffer.h"
#include <memory>

namespace Hazel {

	class VertexArray
	{
	public:
		virtual ~VertexArray() {}

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void AddVertexBuffer(Ref<VertexBuffer>& vertexBuffer) = 0;
		virtual void SetIndexBuffer(Ref<IndexBuffer>& indexBuffer) = 0;
		virtual void SetPrimitiveType(PrimitiveType type) = 0;

		virtual const std::vector<Ref<VertexBuffer>> &GetVertexBuffers() const = 0;
		virtual const Ref<IndexBuffer>& GetIndexBuffer() const = 0;
		virtual const unsigned int GetPrimitiveType() const = 0;

		static VertexArray* Create();

	};

}
