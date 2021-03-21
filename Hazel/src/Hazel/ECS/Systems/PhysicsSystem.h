#pragma once

#include "Hazel/ECS/System.h"
#include "Hazel/ECS//ECSTypes.h"
#include "Hazel/ECS/Coordinator.h"
#include "Hazel/Renderer/Shader.h"
#include "Hazel/Ocean/Ocean.h"

namespace Hazel
{

	// Required components: Transform, RigidBody, PhysicsModel
	class PhysicsSystem : public System
	{
	public:
		void Init(Coordinator* coordinator, Ocean& ocean);
		void OnUpdate(const float& dt);
	private:
		Coordinator* ECS;
		uint32_t _x, _y, _z, _S; // _S the scale of the subocean
		float _scaleY;
		uint32_t SSBO;
		Ref<Shader> cs_FloatingObject = Shader::Create("assets/shaders/compute/BoatTransform.glsl");
	};
}