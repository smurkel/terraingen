#include "Hazel/Hazel.h"
#include "Hazel/Core/EntryPoint.h"

#include "Environments/TestEnvironment.h"

class Sandbox : public Hazel::Application
{
public:
	Sandbox()
	{
		PushLayer(new TestEnvironment());
	}

	~Sandbox()
	{

	}

};

Hazel::Application* Hazel::CreateApplication()
{
	return new Sandbox();
}

