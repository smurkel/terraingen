#pragma once

#include <glm/glm.hpp>

namespace Hazel {

	class Camera
	{
	public:
		Camera(float fov_angle = 45.0f, float aspectratio= 1200.0f/720.0f, float z_near = 0.1f, float z_far = 500.0f);
		
		void SetPosition(glm::vec3& position) { m_Position = position; RecalculateViewMatrix(); }
		void SetFocus(glm::vec3& focus) { m_Focus = focus; }

		void InvertY() { b_InvertY = !b_InvertY; RecalculateViewMatrix(); }
		const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }
		const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
		const glm::mat4& GetViewProjectionMatrix() const { return m_ViewProjectionMatrix; }
		const glm::vec3& GetPositionXYZ() const
		{
			glm::vec3 pos = {
				std::cos(glm::radians(m_Position.y)) * m_Position.x * std::sin(glm::radians(m_Position.z)),
				m_Position.x* std::cos(glm::radians(m_Position.z)),
				std::sin(glm::radians(m_Position.y)) * m_Position.x * std::sin(glm::radians(m_Position.z)),
			};
			return pos;
		}
		const float GetZNear() { return z_Near; }
		const float GetZFar() { return z_Far; }
	private:
		void RecalculateViewMatrix();
	private:
		glm::vec3 m_Position = { 1.0f, 0.0f, 45.0f }; // R, Azimuthal, Polar 
		glm::vec3 m_Focus = { 0.0f, 0.0f, 0.0f }; 

		glm::mat4 m_ProjectionMatrix;
		glm::mat4 m_ViewMatrix;
		glm::mat4 m_ViewProjectionMatrix;

		bool b_InvertY = false;
		float z_Near;
		float z_Far;
	};

}