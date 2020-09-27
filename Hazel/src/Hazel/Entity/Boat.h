#pragma once


#include "Library.h"
#include "Hazel/Core/Timestep.h"
#include "Hazel/Events/KeyEvent.h"

#include "Hazel/Renderer/Renderer.h"
#include "Hazel/Entity/Model.h"

#include "Hazel/Ocean/WaterPhysicsEngine.h"
#include "Hazel/Entity/RigidBody.h"
#include "Hazel/Entity/Entity.h"


namespace Hazel
{

	class Boat: public Entity
	{
	public:
		Boat();
		void Render();
		void OnUpdate(Timestep ts, WaterPhysicsEngine& WPE) override;
		glm::mat4 GetTransform() { return m_Transform; }
	private:

		// rendering
		Ref<Hazel::Model> m_Model;
		Ref<Hazel::Shader> m_Shader;
		// vars
		// A TRICK THAT WORKS WELL TO GET MORE SMOOTH MOVEMENT: MAKE I AND M HERE A FACTOR 10 LARGER THAN THAT OF THE COMBINED RIGID BODY PROBES
		glm::mat3 I = glm::mat3(7.8863, 0.0000, 0.0000, 0.0000, 18.5527, 0.0000, 0.0000, 0.0000, 10.6667);
		float M = 19.5000;
		RigidBody rb = RigidBody(I, M);
		glm::mat4 m_Transform = glm::translate(glm::mat4(rb.R), rb.x);
		// Boat will have a RigidBody object
		// Direction as an angle (used only upon Steer() to calculate the new direction)	

	};

	class BoatController
	{
	public:
		BoatController(Boat& boat);

		Boat& GetBoat() { return m_Boat; }
		void SetBoat(Boat& boat) { m_Boat = boat; }

		void OnEvent(Event& e);
	private:
		Boat& m_Boat;
	};

}