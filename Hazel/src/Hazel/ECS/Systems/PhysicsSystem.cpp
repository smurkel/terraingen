#include "hzpch.h"
#include "PhysicsSystem.h"

#include "Hazel/ECS/Components/ProbeArrayComponent.h"
#include "Hazel/ECS/Components/RigidBody.h"
#include "Hazel/ECS/Components/Transform.h"

#include <glad/glad.h>

namespace Hazel
{
	void PhysicsSystem::Init(Coordinator* coordinator, Ocean& ocean)
	{
		ECS = coordinator;
		_x = ocean.X0;
		_y = ocean.Y0;
		_z = ocean.Z0;
		_S = ocean.GetSuboceanScale(0);
		_scaleY = ocean.GetHeightScaling();

		glGenBuffers(1, &SSBO);
	}
	void PhysicsSystem::OnUpdate(const float& dt)
	{

		cs_FloatingObject->Bind();
		glBindImageTexture(0, _x, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
		glBindImageTexture(1, _y, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
		glBindImageTexture(2, _z, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
		std::dynamic_pointer_cast<OpenGLShader>(cs_FloatingObject)->UploadUniformFloat("u_Scale", (float)_S);
		std::dynamic_pointer_cast<OpenGLShader>(cs_FloatingObject)->UploadUniformFloat("u_HeightScale", _scaleY);
		// per-entity basis
		// i) get rigid body
		// ii) get probe array
		// iii) update probe positions using rigidbody
		// iv) calculate force on probes
		// v) update rigid body using force
		for (Entity entity : mEntities)
		{
			RigidBody& rigidBody = ECS->GetComponent<RigidBody>(entity);
			std::dynamic_pointer_cast<OpenGLShader>(cs_FloatingObject)->UploadUniformFloat3("rb_V", rigidBody.P / rigidBody.m);
			std::dynamic_pointer_cast<OpenGLShader>(cs_FloatingObject)->UploadUniformFloat3("rb_W", rigidBody.w);
			std::dynamic_pointer_cast<OpenGLShader>(cs_FloatingObject)->UploadUniformFloat3("rb_Position", rigidBody.x);

			TransformComponent& transform = ECS->GetComponent<TransformComponent>(entity);
			std::dynamic_pointer_cast<OpenGLShader>(cs_FloatingObject)->UploadUniformMat4("rb_ModelMatrix", transform.matrix);

			ProbeArrayComponent& probeArray = ECS->GetComponent<ProbeArrayComponent>(entity);
			
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO);
			int bufferSize = probeArray.probeArray->GetProbeCount() * sizeof(Probe);
			glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSize, probeArray.probeArray->GetProbes(), GL_DYNAMIC_DRAW);
			GLint bufferMask = GL_MAP_READ_BIT;
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, SSBO);
			glDispatchCompute(1, 1, 1);
			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

			Probe* probeOutput = new Probe[probeArray.probeArray->GetProbeCount()];
			probeOutput = (Probe*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, bufferSize, bufferMask);
			glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

			glm::vec3 Torque(0.0);
			glm::vec3 Force(0.0);
			for (int p = 0; p < probeArray.probeArray->GetProbeCount(); p++)
			{
				Probe temp = *(probeOutput + p);
				Torque += glm::vec3(temp.meta.x, temp.meta.y, temp.meta.z);
				Force += glm::vec3(temp.position.x, temp.position.y, temp.position.z);
			}

			rigidBody.Update(Force, Torque, dt);
			transform.matrix = glm::translate(glm::mat4(rigidBody.R), rigidBody.x);
		}
	}
}