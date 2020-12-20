#include "hzpch.h"
#include "WaterPhysicsEngine.h"
#include <glad/glad.h>

namespace Hazel
{
	WaterPhysicsEngine::WaterPhysicsEngine()
	{
		glGenBuffers(1, &b_SSBO);
	}

	PhysicsProbeOutput WaterPhysicsEngine::FloatingObjectTorque(PhysicsProbe* floaters, int nfloaters, glm::mat4& transform, RigidBody& rb)
	{
		cs_FloatingObjectTransform->Bind();

		std::dynamic_pointer_cast<OpenGLShader>(cs_FloatingObjectTransform)->UploadUniformFloat3("rb_V", rb.P / rb.m);
		std::dynamic_pointer_cast<OpenGLShader>(cs_FloatingObjectTransform)->UploadUniformFloat3("rb_W", rb.w);
		std::dynamic_pointer_cast<OpenGLShader>(cs_FloatingObjectTransform)->UploadUniformFloat3("rb_Pos", rb.x);
		std::dynamic_pointer_cast<OpenGLShader>(cs_FloatingObjectTransform)->UploadUniformMat3("rb_Rot", glm::mat4(rb.R));

		// COMMENTING OUT THESE LINES THAT REFER TO M_OCEAN WHICH (AT 15/12/2020) WAS NO LONGER IN USED - REPLACED BY OCEANHR, WHICH ACTS AS A WRAPPER AROUND 3 DIFFERENT (IN SCALE) OCEAN GEOMETRY CLASSES
		//std::dynamic_pointer_cast<OpenGLShader>(cs_FloatingObjectTransform)->UploadUniformFloat("u_L", m_Ocean->GetL());
		//std::dynamic_pointer_cast<OpenGLShader>(cs_FloatingObjectTransform)->UploadUniformInt("u_N", m_Ocean->GetN());

		glBindImageTexture(0, _x->GetRendererID(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
		glBindImageTexture(1, _y->GetRendererID(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
		glBindImageTexture(2, _z->GetRendererID(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
		for (int i = 0;i < nfloaters;i++)
		{
			PhysicsProbe temp = *(floaters + i);
			//HZ_CORE_INFO("Input Floater {} - xyz {} {} {}, V {}, d {}", i, temp.xyzw.x, temp.xyzw.y, temp.xyzw.z, temp.Vrab.x, temp.Vrab.y);
		}
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, b_SSBO);
		glBufferData(GL_SHADER_STORAGE_BUFFER, nfloaters * sizeof(PhysicsProbe), floaters, GL_DYNAMIC_DRAW);
		GLint bufMask = GL_MAP_READ_BIT;// | GL_MAP_INVALIDATE_BUFFER_BIT;
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, b_SSBO);
		glDispatchCompute(1, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		PhysicsProbe* _floaters = (PhysicsProbe*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, nfloaters * sizeof(PhysicsProbe), bufMask);
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		
		glm::vec3 Torque = { 0.0, 0.0, 0.0 };
		glm::vec3 Force = { 0.0, 0.0, 0.0 };
		int submerged = 0;
		for (int i = 0;i < nfloaters;i++)
		{
			PhysicsProbe temp = *(_floaters + i);
			Torque += glm::vec3(temp.XYZd.x, temp.XYZd.y, temp.XYZd.z);
			Force += glm::vec3(temp.NxNyNzV.x, temp.NxNyNzV.y, temp.NxNyNzV.z);
			//HZ_CORE_INFO("Probe {} output: {}, {}, {}, {}, {}, {}, {}, {}", i, temp.xyzw.x, temp.xyzw.y, temp.xyzw.z, temp.xyzw.w, temp.Vrab.x, temp.Vrab.y, temp.Vrab.z, temp.Vrab.w);
		}
		PhysicsProbeOutput out;
		out.torque = Torque;
		out.force = Force;
		return out;
	}

}
