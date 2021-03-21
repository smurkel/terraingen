#include "BoatController.h"
#include "Hazel/Input.h"
#include "Hazel/Log.h"
#include <glm/gtx/intersect.hpp>

constexpr float PI = 3.141592653589793f;

void BoatController::OnUpdate(float dt, Hazel::Camera& camera)
{
	if (m_ControlActive)
	{
		if (Hazel::Input::IsMouseButtonPressed(1))
		{
			// https://stackoverflow.com/questions/32973060/converting-2d-mouse-coordinates-to-world-xz-coordinates-in-threejs
			// get current cursor pos XY
			glm::vec2 Cursor = glm::vec2((Hazel::Input::GetMouseX() - 600) / 600, (Hazel::Input::GetMouseY() - 360) / 360);
			glm::vec4 NearPlanePosition = glm::vec4(Cursor.x, Cursor.y, 0.0, 1.0);
			glm::vec4 FarPlanePosition = glm::vec4(Cursor.x, Cursor.y, 1.0, 1.0);
			glm::mat4 invVPMat = glm::inverse(camera.GetViewProjectionMatrix());
			NearPlanePosition = invVPMat * NearPlanePosition;
			FarPlanePosition = invVPMat * FarPlanePosition;
			glm::vec3 nearPos = glm::vec3(NearPlanePosition) / NearPlanePosition.w;
			glm::vec3 farPos = glm::vec3(FarPlanePosition) / FarPlanePosition.w;

			glm::vec3 rayDirection = glm::normalize(farPos - nearPos);
			glm::vec3 rayOrigin = camera.GetPositionXYZ();
			glm::vec3 planeOrigin = glm::vec3(0.0f, 0.0f, 1.0f);
			glm::vec3 planeNormal = glm::vec3(0.0f, 1.0f, 0.0f);
			float intersectDistance;
			bool intersect = glm::intersectRayPlane(rayOrigin, rayDirection, planeOrigin, planeNormal, intersectDistance);

			Hazel::BoatParameters& m_CurrentBoatParams = ECS->GetComponent<Hazel::BoatParameters>(m_Entity);
			if (intersect)
			{
				glm::vec3 worldPosition = rayOrigin + intersectDistance * rayDirection;
				glm::vec2 boatPosition = m_CurrentBoatParams.position;
				float newHeading = std::atan2f(boatPosition.y - worldPosition.z, boatPosition.x - worldPosition.x);
				float currentHeading = m_CurrentBoatParams.heading;
				float Step = (newHeading - currentHeading);
				float cstep = Step;
				if (std::abs(Step) > PI)
				{
					Step = ((Step < 0) - (Step > 0)) * (2 * PI - std::abs(Step));
				}
				m_CurrentBoatParams.toturn = Step;
			}
		}
	}
}

