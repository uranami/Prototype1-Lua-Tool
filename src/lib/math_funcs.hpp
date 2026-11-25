#pragma once
#include "matrix.hpp"

namespace arisu::math
{
    float rad_to_deg(float rad);
    float deg_to_rad(float deg);
    float normalize_yaw(float yaw, float range = 360.f);
    void sin_cos(float rad, float& sine, float& cosine);
    void transform(const fVector3& tr, Matrix3x4& matrix, fVector3& out);
    fVector3 transform(fVector3& transform, const Matrix3x4& matrix);
    fVector3 transform(const fVector3& tr, Matrix3x4& matrix);
    std::array<fVector3, 3> angle_vectors(const fAngle& angle);
    void vector_angles(const arisu::fVector3& vec, arisu::fAngle& out);
    float random_float(float min, float max);
    int random_int(int min, int max);
    fAngle calculate_angle(const arisu::fVector3& src, const arisu::fVector3& dest);
}
