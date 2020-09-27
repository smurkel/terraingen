#include "hzpch.h"
#include "CameraController.h"

#include "Hazel/Input.h"
#include "Hazel/Keycodes.h"

namespace Hazel {
	CameraController::CameraController()
	{
	}
	void CameraController::OnUpdate(Timestep ts)
	{
		m_CameraPosition.y += ts.GetSeconds() / 50 * 360;
		m_Camera.SetPosition(m_CameraPosition);
	}

	void CameraController::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<MouseScrolledEvent>(HZ_BIND_EVENT_FN(CameraController::OnMouseScrolled));
		dispatcher.Dispatch<MouseMovedEvent>(HZ_BIND_EVENT_FN(CameraController::OnMouseMoved));
		
	}

	bool CameraController::OnMouseScrolled(MouseScrolledEvent& e)
	{
		m_CameraPosition.x += e.GetYOffset() * m_CameraZoomSpeed;
		m_CameraPosition.x = glm::max(m_CameraPosition.x, m_CameraMaxZoom);
		m_CameraPosition.x = glm::min(m_CameraPosition.x, m_CameraMinZoom);
		m_Camera.SetPosition(m_CameraPosition);
		return false;
	}

	bool CameraController::OnMouseMoved(MouseMovedEvent& e)
	{
		if (Input::IsMouseButtonPressed(1) || Input::IsMouseButtonPressed(2))
		{
			float new_CursorX = Input::GetMouseX();
			float new_CursorY = Input::GetMouseY();

			m_CameraPosition.y += (m_CursorX - new_CursorX) * m_CameraRotationSpeed;
			m_CameraPosition.z += (m_CursorY - new_CursorY) * m_CameraRotationSpeed;
			m_CursorX = new_CursorX;
			m_CursorY = new_CursorY;

			m_CameraPosition.z = glm::min(m_CameraPosition.z, m_CameraMaxPolar);
			m_CameraPosition.z = glm::max(m_CameraPosition.z, m_CameraMinPolar);
			m_Camera.SetPosition(m_CameraPosition);
		}
		else
		{
			m_CursorX = Input::GetMouseX();
			m_CursorY = Input::GetMouseY();
		}
		return false;
	}

}
