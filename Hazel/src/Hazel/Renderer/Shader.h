#pragma once

#include <string>


namespace Hazel {

	class Shader
	{
	public:
		virtual ~Shader() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual const std::string& GetName() const = 0;
		virtual const uint32_t GetRendererID() const = 0;

		static Ref<Shader> Create(const std::string& filepath);
		static Ref<Shader> Create(const std::string name, const std::string& vertexSrc, const std::string& fragmentSrc);
		static Ref<Shader> Create(const std::string name, const std::string& vertexSrc, const std::string& fragmentSrc, const std::string& geometrySrc);
	};

	class ShaderLibrary
	{
	public:
		bool Exists(const std::string& filepath) const;
		Ref<Shader> Get(const std::string& filepath);
		void Add(const std::string& filepath, const Ref<Shader>& shader);
	private:
		std::unordered_map<std::string, Ref<Shader>> m_Shaders;
	};
}