#pragma once

#include "Hazel/Entity/Entity.h"
#include "Hazel/Renderer/Renderer.h"
#include "Hazel/Entity/Model.h"

namespace Hazel
{

	class Island : public Entity
	{
	public:
		Island();
		void Render();
		void OnUpdate(Timestep ts, WaterPhysicsEngine& WPE) override;
		void SetPosition(glm::vec3 position) { m_Position = position; m_Transform = glm::translate(glm::mat4(1.0f), m_Position); }
		glm::vec3 GetPosition() { return m_Position; }
	private:
		glm::vec3 m_Position = glm::vec3(-13.6, -15.4, -8.3);
		glm::mat4 m_Transform = glm::translate(glm::mat4(1.0f), m_Position);
	};

}