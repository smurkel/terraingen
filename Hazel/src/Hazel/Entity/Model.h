#pragma once

#include "Hazel/Renderer/VertexArray.h"
#include "Hazel/Renderer/Renderer.h"
#include <string>
#include <glm/gtc/type_ptr.hpp>
#include "Hazel/Renderer/Texture.h"

namespace Hazel {

	struct ModelVertex 
	{
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 uv;
	};
	struct FaceIndex
	{
		glm::uvec3 v_i;		// vertex index
		glm::uvec3 vt_i;	// vertex texture index
		glm::uvec3 vn_i;	// vertex normal index
	};
	struct PhysicsProbe
	{
		glm::vec4 XYZd;
		glm::vec4 NxNyNzV;
	};
	class Model
	{
	public:
		Model(std::string filepath, std::string texture_filepath);
		~Model();

		Ref<VertexArray> GetVA() { return m_VA; }
		Ref<Texture2D> GetTexture() { return m_Texture; }
		PhysicsProbe* GetProbes() { return m_Probes; }
		uint32_t GetProbeCount() { return m_ProbeCount; }
	private:
		// model data
		PhysicsProbe* m_Probes;
		uint32_t m_ProbeCount;
		ModelVertex* m_Vertices;
		uint32_t* m_Indices;
		Hazel::Ref<Hazel::Texture2D> m_Texture;
		

		// for renderer:
		Hazel::Ref<Hazel::VertexArray> m_VA;
		Hazel::Ref<Hazel::VertexBuffer> m_VB;
		Hazel::Ref<Hazel::IndexBuffer> m_IB;
		
	private:
		bool LoadObj(std::string filepath);
		bool LoadTexture(std::string filepath);
	};

	class ModelLibrary
	{
	public:
		// Whenever the program wants to load a model, it first checks whether it is already loaded:
		inline bool Exists(const std::string& filepath) { return m_Models.find(filepath) != m_Models.end(); }
		// If it does, a reference to that model is returned.
		inline Ref<Model> Get(const std::string filepath) { return m_Models[filepath]; }
		// If not, the model is loaded and added to the library.
		inline void Add(const std::string& filepath, Ref<Model>& model) { m_Models[filepath] = model; }

	private:
		std::unordered_map<std::string, Ref<Model>> m_Models;
	};

}