#include "hzpch.h"
#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Hazel {



	Camera::Camera(float fov_angle, float aspectratio, float z_near, float z_far)
		: m_ProjectionMatrix(glm::perspective(fov_angle, aspectratio, z_near, z_far)), m_ViewMatrix(1.0f), z_Near(z_near), z_Far(z_far)
	{
		RecalculateViewMatrix();
	}

	void Camera::RecalculateViewMatrix()
	{
		if (b_InvertY)
		{
			m_Position.z = 180 - m_Position.z;
			m_Focus.y *= -1.0;
		}
			

		glm::vec3 XYZ = {
			cos(glm::radians(m_Position.y)) * sin(glm::radians(m_Position.z)) * m_Position.x,
			cos(glm::radians(m_Position.z)) * m_Position.x,
			sin(glm::radians(m_Position.y))* sin(glm::radians(m_Position.z))* m_Position.x,
		};

		if (b_InvertY)
		{
			m_Position.z = 180 - m_Position.z;
			m_Focus.y *= -1.0;
		}

		m_ViewMatrix = glm::lookAt(XYZ + m_Focus, m_Focus, glm::vec3(0, 1, 0));
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}



}