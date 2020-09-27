#include "hzpch.h"
#include "Boat.h"

#include "Hazel/Input.h"
#include "Hazel/Keycodes.h"


namespace Hazel
{
	Boat::Boat()
	{
	}

	void Boat::Render()
	{
		Renderer::Submit(this->GetShader(), this->GetModel()->GetVA(), this->GetModel()->GetTexture(), m_Transform);
	}


	void Boat::OnUpdate(Timestep ts, WaterPhysicsEngine& WPE)
	{
		PhysicsProbeOutput net = WPE.FloatingObjectTorque(this->GetModel()->GetProbes(), this->GetModel()->GetProbeCount(), m_Transform, rb);
		rb.Update(net.force, net.torque, ts);
		m_Transform = glm::translate(glm::mat4(rb.R), glm::vec3(rb.x));
	}

	BoatController::BoatController(Boat& boat)
		: m_Boat(boat)
	{

	}

	void BoatController::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
	}
}