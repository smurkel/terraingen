#pragma once

#include <glm/gtc/type_ptr.hpp>

namespace Hazel
{
	struct BoatParameters
	{
		// base
		glm::vec2 position{ 0.0, 0.0 };
		float heading = 0.0f;
		float toturn = 0.0f;

		float velocity = 0.0f;

		float velocityMax = 3.0;
		float turnspeed = 0.1; // degrees per second
		float inertia = 20;
	};
}