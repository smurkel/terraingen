#include "hzpch.h"
#include "RenderSystem.h"


#include "Hazel/ECS/Components/RenderComponents.h"
#include "Hazel/ECS/Components/Transform.h"
#include "Platform/OpenGL/OpenGLShader.h"
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

namespace Hazel
{
	void RenderSystem::RenderEntities(Camera& camera)
	{
		// loop over all the entities to sort by required shader.
		std::unordered_map<uint32_t, std::vector<Entity>> ShaderGroups;
		for (Entity entity : mEntities)
		{
			ShaderComponent shader = ECS->GetComponent<ShaderComponent>(entity);
			ShaderGroups[shader.shader->GetRendererID()].push_back(entity);
		}

		for (auto it = ShaderGroups.begin(); it != ShaderGroups.end(); ++it)
		{
			GLint shaderID = (it->first);
			glUseProgram(shaderID);
			std::string uniformName = "u_ViewProjectionMatrix";
			GLint location = glGetUniformLocation(shaderID, uniformName.c_str());
			glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(camera.GetViewProjectionMatrix()));
			for (Entity entity : it->second)
			{
				_RenderEntity(entity, shaderID);
			}
		}
		glUseProgram(0);
	}

	void RenderSystem::_RenderEntity(Entity entity, int shaderID)
	{
		Ref<Texture> texture = ECS->GetComponent<TextureComponent>(entity).texture;
		Ref<Model> model = ECS->GetComponent<ModelComponent>(entity).model;
		TransformComponent transform = ECS->GetComponent<TransformComponent>(entity);

		std::string uniformName = "u_ModelMatrix";
		GLint location = glGetUniformLocation(shaderID, uniformName.c_str());
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(transform.matrix));
		texture->Bind();
		model->Bind();
		glDrawElements(GL_TRIANGLES, model->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);
	}
}