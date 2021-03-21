#pragma once

#include <string>
#include <glm/gtc/type_ptr.hpp>

namespace Hazel {

	struct Probe
	{
		glm::vec3 position;
		float mass;
		glm::vec4 meta = glm::vec4(0.0);
	};


	class ProbeArray
	{
	public:
		virtual ~ProbeArray() = default;
		virtual const Probe* GetProbes() const = 0;
		virtual uint32_t GetProbeCount() const = 0;
		virtual glm::mat3 GetMomentOfInertia() const = 0;
		virtual float GetMass() const = 0;
		static Ref<ProbeArray> Create(const std::string& filepath);
	};

	class PhysicsProbeArray : public ProbeArray
	{
	public:
		PhysicsProbeArray(const std::string& filename);
		virtual ~PhysicsProbeArray() {};

		virtual const Probe* GetProbes() const override { return m_ProbeArray; }
		virtual uint32_t GetProbeCount() const override { return m_ProbeCount; }
		virtual glm::mat3 GetMomentOfInertia() const override;
		virtual float GetMass() const override;
	private:
		bool LoadObjFile(const std::string& filename);
		glm::mat3 Vec3SelfTransposeMatrix(glm::vec3 V);
		// TODO initialise the probe array with some maximum length, reserving data to enable use of the compoment manager data layout.
		Probe* m_ProbeArray = nullptr;
		float m_ProbeCount = 0;
		glm::mat3 m_Inertia = glm::mat3(1.0);
		float m_Mass = 0;
	};

}