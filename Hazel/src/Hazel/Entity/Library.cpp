#include "hzpch.h"
#include "Model.h"
#include "Library.h"

namespace Hazel
{



	Ref<Model> Library::GetModel(const std::string& filepath, const std::string& texture_filepath)
	{
		if (m_ModelLibrary.Exists(filepath))
		{
			return m_ModelLibrary.Get(filepath);
		}
		else
		{
			Ref<Model> newModel = std::make_shared<Model>(Model::Model(filepath, texture_filepath));
			m_ModelLibrary.Add(filepath, newModel);
			return newModel;
			
		}
	}

	Ref<Shader> Library::GetShader(const std::string& filepath)
	{
		if (m_ShaderLibrary.Exists(filepath))
		{
			return m_ShaderLibrary.Get(filepath);
		}
		else
		{
			Ref<Shader> newShader = Hazel::Shader::Create(filepath);
			m_ShaderLibrary.Add(filepath, newShader);
			return newShader;
		}
	}

}