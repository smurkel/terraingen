#include "Hazel/Hazel.h"
#include "Hazel/Core/EntryPoint.h"

#include "Environments/AtSeaTest.h"

class Sandbox : public Hazel::Application
{
public:
	Sandbox()
	{
		PushLayer(new AtSeaTest());
	}

	~Sandbox()
	{

	}

};

Hazel::Application* Hazel::CreateApplication()
{
	return new Sandbox();
}

