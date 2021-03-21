#include "hzpch.h"
#include "RigidBody.h"
#include <glm/gtx/scalar_multiplication.hpp>

namespace Hazel
{
	RigidBody::RigidBody(float mass, glm::mat3 Ibody)
		: I(glm::inverse(Ibody * 10.0)), m(mass)
	{
	}
	void RigidBody::Update(glm::vec3 F, glm::vec3 T, float dt)
	{
		// F = force, T = torque, dt = interval
		// I update X and R first, then P and L. Not sure if this is the best way to go.
		// source: https://www.cs.cmu.edu/~baraff/sigcourse/notesd1.pdf (page 15)
		x += P / m * dt;
		_I = R * I * glm::transpose(R); // I is already: inverse(Ibody). (see constructor. _I is already I inverse.)
		w = _I * L;
		q += Quaternion(0, (w / 2.0f)) * q * dt;
		R = q.toMatrix();

		P += F * dt;
		L += T * dt;
		P *= 0.99;
		L *= 0.99;
	}

	glm::mat3 RigidBody::Cross(glm::vec3 v)
	{
		return glm::mat3(0, v[2], -v[1], -v[2], 0, v[0], v[1], -v[0], 0);
	}

}