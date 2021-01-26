#pragma once

#include "Hazel/Renderer/Shader.h"
#include "Hazel/Renderer/Buffer.h"
#include "Hazel/Renderer/VertexArray.h"
#include "Hazel/Renderer/Camera.h"
#include "Platform/OpenGL/OpenGLShader.h"

namespace Hazel
{
	class Cubemap
	{
	public:
		Cubemap();
		~Cubemap();

		void LoadTexture(std::vector<std::string> facepath);
		uint32_t GetRendererID() { return m_RendererID; }
	private:
		uint32_t m_RendererID = 0;
		int m_Width = 0;
		int m_Height = 0;
		int m_Channels = 0;
	};
	class Skybox
	{
	public:
		Skybox();
		~Skybox();

		void SetShader(Ref<Shader> shader) { m_Shader = shader; }
		Ref<Shader> GetShader() { return m_Shader; }
		void SetCubemap(int preset_index);

		void Render(Camera& camera);
	private:
		Ref<Shader> m_Shader = 0;
		Cubemap m_Cubemap = Cubemap();

		Hazel::Ref<Hazel::VertexArray> m_VA;
	};

}
