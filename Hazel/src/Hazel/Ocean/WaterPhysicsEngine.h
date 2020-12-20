#pragma once

#include "Ocean.h"
#include "Platform/OpenGL/OpenGLShader.h"

#include "Hazel/Entity/Model.h"
#include "Hazel/Entity/RigidBody.h"

#include <glm/gtc/matrix_transform.hpp>
#include "Hazel/Renderer/Shader.h"

namespace Hazel
{
	struct PhysicsProbeOutput
	{
		glm::vec3 torque;
		glm::vec3 force;
	};
	class WaterPhysicsEngine
	{
	public:
		WaterPhysicsEngine();

		PhysicsProbeOutput FloatingObjectTorque(PhysicsProbe* floaters, int nfloaters, glm::mat4& transform, RigidBody& rb);

		void SetOcean(OceanGeometry* ocean) { m_Ocean = ocean; _x = ocean->GetDisplacementMap(0); _y = ocean->GetDisplacementMap(1); _z = ocean->GetDisplacementMap(2); }
	private:
		Ref<Shader> cs_FloatingObjectTransform = Shader::Create("assets/shaders/compute/BoatTransform.glsl");
		uint32_t b_SSBO;

	public:
		OceanTexture* _x = 0;
		OceanTexture* _y = 0;
		OceanTexture* _z = 0;
		Hazel::OceanGeometry* m_Ocean;
	};

	

}