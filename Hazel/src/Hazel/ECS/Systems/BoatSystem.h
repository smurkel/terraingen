#pragma once

#include "Hazel/ECS/System.h"
#include "Hazel/ECS/ECSTypes.h"
#include "Hazel/ECS/Coordinator.h"
#include <glm/gtc/type_ptr.hpp>

namespace Hazel
{
	// Required components: Transform, BoatParameters
	class BoatSystem : public System
	{
	public:
		void Init(Coordinator* coordinator) { ECS = coordinator; }
		void OnUpdate(float dt, glm::vec2 winddir);
	private:
		Coordinator* ECS;

	};
}

