#pragma once

#include "RenderCommand.h"
#include "Camera.h"
#include "Shader.h"
#include "Hazel/Core/Timestep.h"
#include "Hazel/Entity/Model.h"
#include "Hazel/Renderer/Texture.h"
#include "Hazel/Ocean/Ocean.h"


namespace Hazel {

	class Renderer
	{
	public:
		static void Init();
		static void OnWindowResize(uint32_t width, uint32_t height);

		static void BeginScene(Camera& camera, glm::vec3 lightpos); // TODO
		static void EndScene(); // TODO

		static void Submit(const Ref<Shader>& shader, const Ref<VertexArray>& vertexArray, glm::vec4& c0, glm::vec4& c1, glm::vec4& c2, glm::vec4& c3);
		static void Submit(const Ref<Shader>& shader, const Ref<VertexArray>& vertexArray);
		static void Submit(const Ref<Shader>& shader, const Ref<VertexArray>& vertexArray, const Ref<Texture2D>& texture, const glm::mat4& transform = glm::mat4(1.0f));
		

		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
	private:
		struct SceneData
		{
			glm::mat4 ViewMatrix;
			glm::mat4 ProjectionMatrix;
			glm::vec3 LightPosition;
			glm::vec3 ViewPosition;
		};

		static SceneData* m_SceneData;
	};
	
}
