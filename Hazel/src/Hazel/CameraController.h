#pragma once

#include "Hazel/Renderer/Camera.h"
#include "Hazel/Core/Timestep.h"

#include "Hazel/Events/ApplicationEvent.h"
#include "Hazel/Events/MouseEvent.h"

namespace Hazel {

	class CameraController
	{
	public:
		CameraController();

		void OnUpdate(Timestep ts);
		void OnEvent(Event& e);

		Camera& GetCamera() { return m_Camera; }

		void SetFocus(glm::vec3& focuspos) { m_CameraFocus = focuspos; m_Camera.SetFocus(m_CameraFocus); }
	private:
		bool OnMouseScrolled(MouseScrolledEvent& e);
		bool OnMouseMoved(MouseMovedEvent& e);
	private:
		Camera m_Camera;

		glm::vec3 m_CameraPosition = { 15.0f, 0.0f, 45.0f };
		glm::vec3 m_CameraFocus = { 0.0f, 0.0f, 0.0f };

		float m_CameraTranslationSpeed = 0.2f;
		float m_CameraRotationSpeed = 0.2f;
		float m_CameraZoomSpeed = 1.5f;

		float m_CameraMinZoom = 400.0f;
		float m_CameraMaxZoom = 1.0f;
		float m_CameraMaxPolar = 160.0;
		float m_CameraMinPolar = 15.0;
		
		float m_CursorX = 0.0;
		float m_CursorY = 0.0;
	};

}
