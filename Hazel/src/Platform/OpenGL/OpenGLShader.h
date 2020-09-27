#pragma once

#include "Hazel/Renderer/Shader.h"
#include <glm/glm.hpp>

// TODO: Remove
typedef unsigned int GLenum;

namespace Hazel {

	class OpenGLShader : public Shader
	{
	public:
		OpenGLShader(const std::string& filepath);
		OpenGLShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc);
		OpenGLShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc, const std::string& geometrySrc);
		virtual ~OpenGLShader();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual const std::string& GetName() const override { return m_Name; }
		virtual const uint32_t GetRendererID() const override { return m_RendererID; }

		void UploadUniformFloat4(const std::string name, const glm::vec4& values);
		void UploadUniformFloat3(const std::string name, const glm::vec3& values);
		void UploadUniformFloat2(const std::string name, const glm::vec2& values);
		void UploadUniformFloat(const std::string name, float values);

		void UploadUniformInt(const std::string name, int value);

		void UploadUniformMat4(const std::string name, const glm::mat4& matrix);
		void UploadUniformMat3(const std::string name, const glm::mat3& matrix);

	private:
		std::string ReadFile(const std::string& filepath);
		std::unordered_map<GLenum, std::string> PreProcess(const std::string& source);
		void Compile(const std::unordered_map<GLenum, std::string>& shaderSources);
	private:
		uint32_t m_RendererID;
		std::string m_Name;
	};

}