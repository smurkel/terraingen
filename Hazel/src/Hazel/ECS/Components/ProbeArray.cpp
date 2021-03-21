#include "hzpch.h"
#include "ProbeArray.h"

#include <fstream>

namespace Hazel
{

	Ref<ProbeArray> ProbeArray::Create(const std::string& filepath)
	{
		return std::make_shared<PhysicsProbeArray>(filepath);
	}

	PhysicsProbeArray::PhysicsProbeArray(const std::string& filepath)
	{
		LoadObjFile(filepath);
	}

	glm::mat3 PhysicsProbeArray::GetMomentOfInertia() const
	{
		return m_Inertia;
	}

	float PhysicsProbeArray::GetMass() const
	{
		return m_Mass;
	}

	bool PhysicsProbeArray::LoadObjFile(const std::string& filepath)
	{
		std::vector<Probe> _probes;
		m_ProbeCount = 0;
		char* path = new char[filepath.length() + 1];
		std::strcpy(path, filepath.c_str());
		FILE* file = fopen(path, "r");
		if (file == NULL)
		{
			HZ_CORE_ASSERT(0, "File can not be opened");
			return false;
		}

		while (1)
		{
			char lineHeader[128];
			int res = fscanf(file, "%s", lineHeader);
			if (res == EOF)
				break;

			if (strcmp(lineHeader, "pr") == 0)
			{
				Probe probe;
				int temp = fscanf(file, "%f %f %f %f", &probe.position.x, &probe.position.y, &probe.position.z, &probe.mass);
				HZ_CORE_INFO("{0}", lineHeader);
				_probes.push_back(probe);
				m_ProbeCount++;
			}
		}
		m_ProbeArray = new Probe[m_ProbeCount];
		glm::vec3 COM(0.0);
		for (int p = 0; p < m_ProbeCount; p++)
		{
			Probe P = _probes[p];
			m_ProbeArray[p] = P;
			m_Mass += P.mass;
			COM += P.mass * P.position;
		}
		COM /= m_Mass;

		glm::mat3 imat = glm::mat3(1.0);
		// calculate mass and center of mass
		for (int p = 0; p < m_ProbeCount; p++)
		{
			Probe P = m_ProbeArray[p];
			glm::vec3 X = P.position;
			float m = P.mass;
			m_Inertia += P.mass * (imat * (glm::dot(X, X))	 - Vec3SelfTransposeMatrix(X));
		}
		HZ_CORE_INFO("Mass, center of mass: {0}, ({1}, {2}, {3})", m_Mass, COM.x, COM.y, COM.z);
		
		HZ_CORE_INFO("Moment of inertia: \n{0}\t{1}\t{2}\n{3}\t{4}\t{5}\n{6}\t{7}\t{8}", m_Inertia[0][0], m_Inertia[0][1], m_Inertia[0][2], m_Inertia[1][0], m_Inertia[1][1], m_Inertia[1][2], m_Inertia[2][0], m_Inertia[2][1], m_Inertia[2][2]);
	}

	glm::mat3 PhysicsProbeArray::Vec3SelfTransposeMatrix(glm::vec3 V)
	{
		float x2 = V.x * V.x;
		float y2 = V.y * V.y;
		float z2 = V.z * V.z;
		float xy = V.x * V.y;
		float xz = V.x * V.z;
		float yz = V.y * V.z;
		return glm::mat3(x2, xy, xz, xy, y2, yz, xz, yz, z2);
	}

}