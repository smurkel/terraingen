#pragma once

#include <string>
#include <Hazel/Renderer/VertexArray.h>
#include <glm/gtc/type_ptr.hpp>

namespace Hazel {

	struct FaceIndex
	{
		glm::uvec3 v_i;
		glm::uvec3 vt_i;
		glm::uvec3 vn_i;
	};
	struct ModelVertex
	{
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 uv;
	};

	class Model
	{
	public:
		virtual ~Model() = default;
		virtual void Bind() const = 0;
		virtual const Ref<IndexBuffer>& GetIndexBuffer() const = 0;
		static Ref<Model> Create(const std::string& filepath);
	};

	class Model3D : public Model
	{
	public:
		Model3D(const std::string& filename);
		virtual ~Model3D() {};

		virtual const Ref<IndexBuffer>& GetIndexBuffer() const override { return m_VertexArray->GetIndexBuffer(); }
		virtual void Bind() const override;
	private:
		bool LoadObjFile(const std::string& filename);
		Ref<VertexArray> m_VertexArray;
	};
}