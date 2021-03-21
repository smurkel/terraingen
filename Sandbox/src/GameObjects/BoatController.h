#pragma once

#include "Hazel/ECS/Components/BoatParams.h"
#include "Hazel/ECS/Coordinator.h"
#include "Hazel/Renderer/Camera.h"

#include <glm/gtc/type_ptr.hpp>


class BoatController
{
public:
	BoatController() {}
	void Init(Hazel::Coordinator* ecs) { ECS = ecs; }
	void OnUpdate(float dt, Hazel::Camera& camera);
	void SetCurrentBoat(Hazel::Entity entity) { m_Entity = entity; }
	Hazel::Entity GetCurrentBoat() { return m_Entity; }
	void SetActive() { m_ControlActive = true; }
	void SetInactive() { m_ControlActive = false; }
private:
	Hazel::Entity m_Entity;
	Hazel::Coordinator* ECS;
	bool m_ControlActive = false;
};
