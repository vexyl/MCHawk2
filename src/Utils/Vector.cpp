// Learned from Math for Game Developers youtube series by Jorge Rodriguez
#include "../../include/Utils/Vector.hpp"

#include <assert.h>
#include <math.h>

namespace Utils {
Vector::Vector(float x, float y, float z)
{
	this->x = x;
	this->y = y;
	this->z = z;
}

Vector::Vector(const Vector& v)
{
	x = v.x;
	y = v.y;
	z = v.z;
}

float Vector::Length() const
{
	double tx = static_cast<double>(x);
	double ty = static_cast<double>(y);
	double tz = static_cast<double>(z);
	return static_cast<float>((sqrt((tx*tx + ty*ty + tz*tz))));
}

float Vector::LengthSqr() const
{
	return (x*x + y*y + z*z);
}

Vector Vector::operator=(const Vector& v)
{
	x = v.x;
	y = v.y;
	z = v.z;
	return *this;
}

Vector Vector::Multiply(Vector v) const
{
	return Vector(x * v.x, y * v.y, z * v.z);
}

Vector Vector::Multiply(float s) const
{
	return Vector(x * s, y * s, z * s);
}

Vector Vector::Divide(float s) const
{
	assert(s != 0);
	return Vector(x / s, y / s, z / s);
}

Vector Vector::Add(const Vector& v) const
{
	return Vector(x + v.x, y + v.y, z + v.z);
}

Vector Vector::Subtract(const Vector& v) const
{
	return Vector(x - v.x, y - v.y, z - v.z);
}

Vector Vector::operator*(Vector v) const
{
	return Vector(x * v.x, y * v.y, z * v.z);
}

Vector Vector::operator*(float s) const
{
	return Vector(x * s, y * s, z * s);
}

Vector Vector::operator/(float s) const
{
	assert(s != 0);
	return Vector(x / s, y / s, z / s);
}

Vector Vector::operator+(const Vector& v) const
{
	return Vector(x + v.x, y + v.y, z + v.z);
}

Vector Vector::operator-(const Vector& v) const
{
	return Vector(x - v.x, y - v.y, z - v.z);
}

Vector Vector::operator+=(const Vector& v)
{
	x += v.x;
	y += v.y;
	z += v.z;
	return *this;
}

Vector Vector::operator-=(const Vector& v)
{
	x -= v.x;
	y -= v.y;
	z -= v.z;
	return *this;
}

Vector Vector::Normalized() const
{
	return *this / Length();
}

Vector Vector::Cross(const Vector& v) const
{
	return Vector((this->y * v.z) - (this->z * v.y), (this->z * v.x) - (this->x * v.z), (this->x * v.y) - (this->y * v.x));
}

float Vector::DotProduct(const Vector& v) const
{
	return this->x * v.y + this->y * v.y + this->z * v.z;
}
} // namespace Utils
