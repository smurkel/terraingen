#include "hzpch.h"
#include "Quaternion.h"

namespace Hazel
{

	Quaternion::Quaternion() {}
	Quaternion::Quaternion(float _s, glm::vec3 _v)
	{
		s = _s;
		v = _v;
	}

	void Quaternion::operator+=(const Quaternion& q)
	{
		s += q.s;
		v += q.v;
	}

	Quaternion Quaternion::operator+(const Quaternion& q) const
	{
		float scalar = s + q.s;
		glm::vec3 imaginary = v + q.v;
		return Quaternion(scalar, imaginary);
	}

	void Quaternion::operator-=(const Quaternion& q)
	{
		s -= q.s;
		v -= q.v;
	}

	Quaternion Quaternion::operator-(const Quaternion& q) const
	{
		float scalar = s - q.s;
		glm::vec3 imaginary = v - q.v;
		return Quaternion(scalar, imaginary);
	}

	void Quaternion::operator*=(const Quaternion& q)
	{
		(*this) = multiply(q);
	}

	Quaternion Quaternion::operator*(const Quaternion& q) const
	{
		float scalar = s * q.s - glm::dot(v, q.v);
		glm::vec3 imaginary = q.v * s + v * q.s + glm::cross(v, q.v);
		return Quaternion(scalar, imaginary);
	}

	Quaternion Quaternion::multiply(const Quaternion& q) const
	{
		float scalar = s * q.s - glm::dot(v, q.v);
		glm::vec3 imaginary = q.v * s + v * q.s + glm::cross(v, q.v);
		return Quaternion(scalar, imaginary);
	}

	void Quaternion::operator*=(const float value)
	{
		s *= value;
		v *= value;
	}

	Quaternion Quaternion::operator*(const float value) const
	{
		float scalar = s * value;
		glm::vec3 imaginary = v * value;
		return Quaternion(scalar, imaginary);
	}

	float Quaternion::norm()
	{
		float scalar = s * s;
		float imaginary = v.x * v.x + v.y * v.y + v.z * v.z;
		return std::sqrt(scalar + imaginary);
	}

	void Quaternion::normalize()
	{
		if (norm() != 0)
		{
			float normValue = 1 / norm();
			s *= normValue;
			v *= normValue;
		}
	}

	void Quaternion::convertToUnitNormQuaternion()
	{
		float angle = s * 2.0 * M_PI / 360.0;
		v = glm::normalize(v);
		s = cosf(angle * 0.5);
		v = v * sinf(angle * 0.5);
	}

	Quaternion Quaternion::conjugate()
	{
		float scalar = s;
		glm::vec3 imaginary = -v;
		return Quaternion(scalar, imaginary);
	}

	Quaternion Quaternion::inverse()
	{
		float absoluteValue = norm();
		absoluteValue *= absoluteValue;
		absoluteValue = 1 / absoluteValue;
		Quaternion conjugateValue = conjugate();
		float scalar = conjugateValue.s * absoluteValue;
		glm::vec3 imaginary = conjugateValue.v * absoluteValue;
		return Quaternion(scalar, imaginary);
	}

	glm::vec3 Quaternion::rotateVectorAboutAngleAndAxis(glm::vec3 vector, float uAngle, glm::vec3& uAxis)
	{
		Quaternion p(0, vector);
		glm::normalize(uAxis);
		Quaternion q(uAngle, uAxis);
		q.convertToUnitNormQuaternion();
		Quaternion qInverse = q.inverse();
		Quaternion rotatedVector = q * p * qInverse;
		return rotatedVector.v;
	}

	glm::mat3 Quaternion::toMatrix()
	{
		normalize();
		float x = v[0];
		float x2 = x * x;
		float y = v[1];
		float y2 = y * y;
		float z = v[2];
		float z2 = z * z;
		return glm::mat3(1 - 2 * y2 - 2 * z2, 2 * x * y + 2 * s * z, 2 * x * z - 2 * s * y, 2 * x * y - 2 * s * z, 1 - 2 * x2 - 2 * z2, 2 * y * z + 2 * s * x, 2 * x * z + 2 * s * y, 2 * y * z - 2 * s * x, 1 - 2 * x2 - 2 * y2);

	}

}