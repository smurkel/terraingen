#include "hzpch.h"
#include "BoatSystem.h"

#include "Hazel/ECS/Components/BoatParams.h"
#include "Hazel/ECS/Components/Transform.h"

namespace Hazel
{
	void BoatSystem::OnUpdate(float dt, glm::vec2 wind)
	{
		for (Entity entity : mEntities)
		{
			BoatParameters& bp = ECS->GetComponent<BoatParameters>(entity);
			//HZ_CORE_INFO("Boat heading: {0}", bp.heading);
			glm::vec2 direction = glm::vec2(glm::cos(bp.heading), -glm::sin(bp.heading));
			bp.velocity += glm::dot(direction, wind) * dt / bp.inertia;
			bp.velocity = glm::min(bp.velocity, bp.velocityMax);
			bp.velocity = 2.0;
			if (bp.toturn != 0.0f)
			{
				float turnstep = bp.turnspeed * dt;
				float absmax = std::abs(bp.toturn);
				turnstep = turnstep * (turnstep < absmax) + absmax * (turnstep > absmax);
				float turnsign = (bp.toturn > 0) - (bp.toturn < 0);

				float toTurn = turnsign * turnstep;
				bp.heading += turnsign * turnstep;
				bp.toturn -= toTurn;
				bp.heading = std::fmodf(bp.heading, 2.0f * M_PI);
			}
			
			bp.position += bp.velocity * direction * dt;
			TransformComponent& tc = ECS->GetComponent<TransformComponent>(entity);
			HZ_CORE_INFO("Direction = {0}, {1}", direction.x, direction.y);
			tc.matrix = glm::rotate(glm::translate(glm::mat4(1.0), glm::vec3(bp.position.x, 0.0, bp.position.y)), bp.heading, glm::vec3(0.0, 1.0, 0.0));
			//tc.matrix = glm::translate(glm::rotate(glm::mat4(1.0), bp.heading, glm::vec3(0.0, 1.0, 0.0)), glm::vec3(bp.position.x, 0.0, bp.position.y));

		}
	}
}
