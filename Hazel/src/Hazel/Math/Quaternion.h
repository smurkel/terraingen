#pragma once

#include <glm/gtc/type_ptr.hpp>

namespace Hazel
{

	class Quaternion
	{
	public:
		float s;
		glm::vec3 v;

	public:
		Quaternion();
		Quaternion(float _s, glm::vec3 _v);
		
		void operator+=(const Quaternion& q);
		Quaternion operator+(const Quaternion& q) const;
		void operator-=(const Quaternion& q);
		Quaternion operator-(const Quaternion& q) const;
		void operator*=(const Quaternion& q);
		Quaternion operator*(const Quaternion& q) const;
		Quaternion multiply(const Quaternion& q) const;
		void operator*=(const float value);
		Quaternion operator*(const float value) const;
		float norm();
		void normalize();
		void convertToUnitNormQuaternion();
		Quaternion conjugate();
		Quaternion inverse();
		glm::vec3 rotateVectorAboutAngleAndAxis(glm::vec3 vector, float uAngle, glm::vec3& uAxis);
		glm::mat3 toMatrix();
	};


}