#pragma once

#include <glm/gtc/type_ptr.hpp>
#include "Hazel/Math/Quaternion.h"


namespace Hazel
{
	class RigidBody
	// Physics implementation only! Leave rendering and such to the owner.
	{
	public:
		RigidBody(glm::mat3 Ibody, float mass);
		void Update(glm::vec3 force, glm::vec3 torque, float dt);
		glm::vec3 GetPosition() { return x; }
		glm::mat3 GetOrientation() { return R; }
	private:
		glm::mat3 Cross(glm::vec3 v);
	public:
		// State parameters
		glm::vec3	x = { 0.0, 0.5, 0.0 };	// Point position
		glm::vec3	P = { 0.0, 0.0, 0.0 };	// Linear momentum
		glm::mat3	R = glm::mat3(1.0f);	// Orientation
		Quaternion q = Quaternion(1, glm::vec3(0, 0, 0));
		glm::vec3	L = glm::vec3(0.0f);	// Angular momentum
		float		m;						// Mass
		glm::mat3	I;						// Inverse of moment of inertia matrix in body coordinates
		// External & auxiliary
		glm::mat3 _I = glm::mat3(1.0f);
		glm::vec3 w = glm::vec3(0.0f);

		
	};

}