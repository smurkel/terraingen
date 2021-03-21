#pragma once

#include <glm/gtc/type_ptr.hpp>
#include "Hazel/Math/Quaternion.h"

namespace Hazel
{

	//class IRigidBody
	//{
	//public:
	//	virtual ~IRigidBody() = default;
	//	virtual void Update(glm::vec3 force, glm::vec3 torque, float dt) const = 0;
	//	//virtual void SetMass(float mass) const = 0;
	//	//virtual void SetInertia(glm::mat3 inertia) const = 0;
	//	virtual Ref<IRigidBody> Create(float mass, glm::mat3 inertia);
	//};
	class RigidBody
	{
	public:
		RigidBody() = default;
		RigidBody(float mass, glm::mat3 Ibody);
		void Update(glm::vec3 force, glm::vec3 torque, float dt);
	private:
		glm::mat3 Cross(glm::vec3 v);
	public:
		// State parameters
		glm::vec3	x = { 0.0, 0.5, 0.0 };	// Point position
		glm::vec3	P = { 0.0, 0.0, 0.0 };	// Linear momentum
		glm::mat3	R = glm::mat3(1.0f);	// Orientation
		glm::vec3	L = glm::vec3(0.0f);	// Angular momentum
		float		m = 0;						// Mass
		glm::mat3	I{};						// Inverse of moment of inertia matrix in body coordinates
		Quaternion q = Quaternion(1, glm::vec3(0, 0, 0));
		// Auxiliary
		glm::mat3 _I = glm::mat3(1.0f);
		glm::vec3 w = glm::vec3(0.0f);
	};

}