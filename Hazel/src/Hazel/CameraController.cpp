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
		//m_CameraPosition.y += ts.GetSeconds() / 50 * 360;
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
		if (Input::IsMouseButtonPressed(2))
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
		else if (Input::IsMouseButtonPressed(0))
		{
			float new_CursorX = Input::GetMouseX();
			float new_CursorY = Input::GetMouseY(); 
			
			glm::mat3 R = glm::mat3(m_Camera.GetViewMatrix());
			glm::vec3 dC = -glm::vec3(new_CursorX - m_CursorX, 0.0, -(new_CursorY - m_CursorY));

			glm::vec3 dF = R * dC * m_CameraTranslationSpeed;
			int shiftPressed = Input::IsKeyPressed(HZ_KEY_LEFT_SHIFT);
			HZ_CORE_INFO("Shift pressed: {0}", shiftPressed);
			HZ_CORE_INFO("dF = {0}, {1}, {2}", dF.x, dF.y, dF.z);
			m_CameraFocus.x += (1 - shiftPressed) * dF.x;
			m_CameraFocus.z += (1 - shiftPressed) * dF.y;
			m_CameraFocus.y += shiftPressed * dC.z * m_CameraTranslationSpeed;
			
			
			m_Camera.SetFocus(m_CameraFocus);
			m_CursorX = new_CursorX;
			m_CursorY = new_CursorY;

		}
		else
		{
			m_CursorX = Input::GetMouseX();
			m_CursorY = Input::GetMouseY();
		}
		return false;
	}

}
