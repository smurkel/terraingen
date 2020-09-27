#include "hzpch.h"
#include "Renderer.h"

#include "Platform/OpenGL/OpenGLShader.h"
#include <glfw/glfw3.h>

#include <glm/gtx/string_cast.hpp>

namespace Hazel {

	Renderer::SceneData* Renderer::m_SceneData = new Renderer::SceneData;

	void Renderer::Init()
	{
		RenderCommand::Init();
	}

	void Renderer::OnWindowResize(uint32_t width, uint32_t height)
	{
		RenderCommand::SetViewport(0, 0, width, height);
	}

	void Renderer::BeginScene(Camera& camera, glm::vec3 lightpos)
	{
		m_SceneData->ViewMatrix = camera.GetViewMatrix();
		m_SceneData->ProjectionMatrix = camera.GetProjectionMatrix();
		m_SceneData->LightPosition = lightpos;
		m_SceneData->ViewPosition = camera.GetPositionXYZ();
	}

	void Renderer::EndScene()
	{
	}

	void Renderer::Submit(const Ref<Shader>& shader, const Ref<VertexArray>& vertexArray, glm::vec4& c0, glm::vec4& c1, glm::vec4& c2, glm::vec4& c3)
	{
		// Used for untextured models (many Entities, e.g. Sea).
		shader->Bind();

		std::dynamic_pointer_cast<OpenGLShader>(shader)->UploadUniformFloat4("C_Emissive", c0);
		std::dynamic_pointer_cast<OpenGLShader>(shader)->UploadUniformFloat4("C_Ambient", c1);
		std::dynamic_pointer_cast<OpenGLShader>(shader)->UploadUniformFloat4("C_Diffuse", c2);
		std::dynamic_pointer_cast<OpenGLShader>(shader) ->UploadUniformFloat4("C_Specular", c3);
		std::dynamic_pointer_cast<OpenGLShader>(shader)->UploadUniformMat4("u_ViewMatrix", m_SceneData->ViewMatrix);
		std::dynamic_pointer_cast<OpenGLShader>(shader)->UploadUniformMat4("u_ProjectionMatrix", m_SceneData->ProjectionMatrix);
		//std::dynamic_pointer_cast<OpenGLShader>(shader)->UploadUniformMat4("u_Transform", transform);
		std::dynamic_pointer_cast<OpenGLShader>(shader)->UploadUniformFloat3("u_LightPosition", m_SceneData->LightPosition);
		std::dynamic_pointer_cast<OpenGLShader>(shader)->UploadUniformFloat3("u_ViewPosition", m_SceneData->ViewPosition);

		vertexArray->Bind();
		RenderCommand::DrawIndexed(vertexArray);
	}

	void Renderer::Submit(const Ref<Shader>& shader, const Ref<VertexArray>& vertexArray, const Ref<Texture2D>& texture, const glm::mat4& transform)
	{
		// Used for textured models (many Entities, e.g. Boat).
		shader->Bind();
		texture->Bind();
		std::dynamic_pointer_cast<OpenGLShader>(shader)->UploadUniformMat4("u_ViewMatrix", m_SceneData->ViewMatrix);
		std::dynamic_pointer_cast<OpenGLShader>(shader)->UploadUniformMat4("u_ProjectionMatrix", m_SceneData->ProjectionMatrix);
		std::dynamic_pointer_cast<OpenGLShader>(shader)->UploadUniformMat4("u_Transform", transform);
		std::dynamic_pointer_cast<OpenGLShader>(shader)->UploadUniformFloat3("u_LightPosition", m_SceneData->LightPosition);
		std::dynamic_pointer_cast<OpenGLShader>(shader)->UploadUniformFloat3("u_ViewPosition", m_SceneData->ViewPosition);

		vertexArray->Bind();
		RenderCommand::DrawIndexed(vertexArray);
	}

}