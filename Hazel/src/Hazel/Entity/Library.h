#pragma once

#include "Model.h"
#include "Hazel/Renderer/Shader.h"
#include <string>

namespace Hazel
{

	class Library
	{
		// Library object stores lists of models and shaders.
		// Model stuff:
	public:
		Ref<Model> GetModel(const std::string& filepath, const std::string& texture_filepath);
		Ref<Shader> GetShader(const std::string& filepath);
		
	private:
		ModelLibrary m_ModelLibrary;
		ShaderLibrary m_ShaderLibrary;
	};

}