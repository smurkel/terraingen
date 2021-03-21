#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include "Hazel/Renderer/Shader.h"
#include "Hazel/Renderer/Texture.h"
#include "Hazel/Renderer/Model.h"

namespace Hazel
{
	struct ShaderComponent
	{
		Ref<Shader> shader;
	};

	struct ModelComponent
	{
		Ref<Model> model;
	};

	struct TextureComponent
	{
		Ref<Texture2D> texture;
	};
}