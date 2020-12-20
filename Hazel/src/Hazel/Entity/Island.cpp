#include "hzpch.h"
#include "Island.h"

namespace Hazel
{

	Island::Island()
	{
	}

	void Island::Render()
	{
		Renderer::Submit(this->GetShader(), this->GetModel()->GetVA(), this->GetModel()->GetTexture(), m_Transform);
	}

	void Island::OnUpdate(Timestep ts, WaterPhysicsEngine& WPE)
	{
	}

}


