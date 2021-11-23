#ifndef VECTOR_H_
#define VECTOR_H_

// Learned from Math for Game Developers youtube series by Jorge Rodriguez

namespace Utils {
struct Vector {
	float x, y, z;

	Vector() : x(0.0f), y(0.0f), z(0.0f) { }
	Vector(float x, float y, float z);
	Vector(const Vector& v);

	float Length() const;
	float LengthSqr() const;

	Vector operator=(const Vector& v);
	Vector operator*(Vector v) const;
	Vector operator*(float s) const;
	Vector operator/(float s) const;
	Vector operator+(const Vector& v) const;
	Vector operator-(const Vector& v) const;
	Vector operator+=(const Vector& v);
	Vector operator-=(const Vector& v);

	Vector Normalized() const;
	Vector Cross(const Vector& v) const;
	float DotProduct(const Vector& v) const;
};
} // namespace Utils

#endif // VECTOR_H_
