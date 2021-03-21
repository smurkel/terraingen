#pragma once

#include "Hazel/ECS/System.h"
#include "Hazel/ECS/ECSTypes.h"
#include "Hazel/Renderer/Camera.h"
#include "Hazel/ECS/Coordinator.h"

namespace Hazel
{

	// Required components: Transform, VA, Texture, Shader
	class RenderSystem : public System
	{
	public:
		void Init(Coordinator* coordinator) { ECS = coordinator; }
		void RenderEntities(Camera& camera);
	private:
		void _RenderEntity(Entity entity, int shaderID);
		Coordinator* ECS;
	};
} 