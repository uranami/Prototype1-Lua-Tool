#include "angle.hpp"
#include "vectors.hpp"
#include "math_funcs.hpp"

#include "matrix.hpp"

#include <random>

float arisu::math::rad_to_deg(const float rad)
{
    return rad * (180.f / std::numbers::pi_v<float>);
}

float arisu::math::deg_to_rad(const float deg)
{
    return deg * (std::numbers::pi_v<float> / 180.f);
}

float arisu::math::normalize_yaw(const float yaw, const float range)
{
    return std::isfinite(yaw) ? std::remainderf(yaw, range) : 0.f;
}

void arisu::math::sin_cos(const float rad, float& sine, float& cosine)
{
    sine   = std::sin(rad);
    cosine = std::cos(rad);
}

void arisu::math::transform(const fVector3& tr, Matrix3x4& matrix, fVector3& out)
{
    out.x = tr.dot_product(matrix.at(0)) + matrix.at(0, 3);
    out.y = tr.dot_product(matrix.at(1)) + matrix.at(1, 3);
    out.z = tr.dot_product(matrix.at(2)) + matrix.at(2, 3);
}

arisu::fVector3 arisu::math::transform(fVector3& transform, const Matrix3x4& matrix)
{
    return fVector3(

        transform.dot_product(matrix[0]) + matrix[0][3],
        transform.dot_product(matrix[1]) + matrix[1][3],
        transform.dot_product(matrix[2]) + matrix[2][3]

    );

}

arisu::fVector3 arisu::math::transform(const fVector3& tr, Matrix3x4& matrix)
{
    fVector3 out;
    out.x = tr.dot_product(matrix.at(0)) + matrix.at(0, 3);
    out.y = tr.dot_product(matrix.at(1)) + matrix.at(1, 3);
    out.z = tr.dot_product(matrix.at(2)) + matrix.at(2, 3);
    return out;
}

float arisu::math::random_float(const float min, const float max)
{
    static std::random_device rd;
    static std::mt19937 e2(rd());
    std::uniform_real_distribution dist(min, max);

    return dist(e2);
}

int arisu::math::random_int(const int min, const int max)
{
    static std::random_device rd;
    static std::mt19937 e2(rd());
    std::uniform_int_distribution dist(min, max);

    return dist(e2);
}

arisu::fAngle arisu::math::calculate_angle(const arisu::fVector3& src, const arisu::fVector3& dest)
{
    fAngle angles;
    const auto delta = dest - src;
    vector_angles(delta, angles);

    return angles.normalize();
}

std::array<arisu::fVector3, 3> arisu::math::angle_vectors(const arisu::fAngle& angle)
{
    std::array<arisu::fVector3, 3> results;

    auto sine = arisu::fVector3(), cosine = arisu::fVector3();

    sin_cos(deg_to_rad(angle.x), sine.x, cosine.x);
    sin_cos(deg_to_rad(angle.y), sine.y, cosine.y);
    sin_cos(deg_to_rad(angle.z), sine.z, cosine.z);

    results[0].x = cosine.x * cosine.y;
    results[0].y = cosine.x * sine.y;
    results[0].z = -sine.x;
    results[0]  = results[0].normalized();

    results[1].x = -1 * sine.z * sine.x * cosine.y + -1 * cosine.z * -sine.y;
    results[1].y = -1 * sine.z * sine.x * sine.y + -1 * cosine.z * cosine.y;
    results[1].z = -1 * sine.z * cosine.x;
    results[1]   = results[1].normalized();

    results[2].x = cosine.z * sine.x * cosine.y + -sine.z * -sine.y;
    results[2].y = cosine.z * sine.x * sine.y + -sine.z * cosine.y;
    results[2].z = cosine.z * cosine.x;
    results[2]   = results[2].normalized();
    return results;
}

void  arisu::math::vector_angles(const arisu::fVector3& vec, arisu::fAngle& out)
{
    float flPitch, flYaw;

    if (vec.x == 0.f && vec.y == 0.f)
    {
        flPitch = vec.z > 0.f ? 270.f : 90.f;
        flYaw = 0.f;
    }
    else
    {
        flPitch = std::atan2f(-vec.z, vec.length2d()) * 180.f / std::numbers::pi_v<float>;

        if (flPitch < 0.f)
            flPitch += 360.f;

        flYaw = std::atan2f(vec.y, vec.x) * 180.f / std::numbers::pi_v<float>;

        if (flYaw < 0.f)
            flYaw += 360.f;
    }

    out.x = flPitch;
    out.y = flYaw;
    out.z = 0.f;
}
