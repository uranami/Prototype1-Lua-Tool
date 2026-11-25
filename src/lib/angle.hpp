#pragma once

#include <numbers>
#include <type_traits>
#include <algorithm>
#include <cmath>

namespace arisu
{
    namespace impl
    {
        template <typename T>
        class Vector3;

        template <typename T>
        class Angle
        {
          public:
            T x, y, z;

            Angle(T x_ = 0, T y_ = 0, T z_ = 0)
             : x(x_),
               y(y_),
               z(z_)
            {
            }

            Angle(const T* arr)
             : x(arr[0]),
               y(arr[1]),
               z(arr[2])
            {
            }

            constexpr Angle& operator=(const Angle& base)
            {
                x = base.x;
                y = base.y;
                z = base.z;
                return *this;
            }

            constexpr Angle& operator+=(const Angle& base)
            {
                x += base.x;
                y += base.y;
                z += base.z;
                return *this;
            }

            constexpr Angle& operator-=(const Angle& base)
            {
                x -= base.x;
                y -= base.y;
                z -= base.z;
                return *this;
            }

            constexpr Angle& operator*=(const Angle& base)
            {
                x *= base.x;
                y *= base.y;
                z *= base.z;
                return *this;
            }

            constexpr Angle& operator/=(const Angle& base)
            {
                x /= base.x;
                y /= base.y;
                z /= base.z;
                return *this;
            }

            constexpr Angle& operator+=(const T base)
            {
                x += base;
                y += base;
                z += base;
                return *this;
            }

            constexpr Angle& operator-=(const T base)
            {
                x -= base;
                y -= base;
                z -= base;
                return *this;
            }

            constexpr Angle& operator*=(const T base)
            {
                x *= base;
                y *= base;
                z *= base;
                return *this;
            }

            constexpr Angle& operator/=(const T base)
            {
                x /= base;
                y /= base;
                z /= base;
                return *this;
            }

            Angle operator+(const Angle& base) const
            {
                return { x + base.x, y + base.y, z + base.z };
            }

            Angle operator-(const Angle& base) const
            {
                return { x - base.x, y - base.y, z - base.z };
            }

            Angle operator*(const Angle& base) const
            {
                return { x * base.x, y * base.y, z * base.z };
            }

            Angle operator/(const Angle& base) const
            {
                return { x / base.x, y / base.y, z / base.z };
            }

            Angle operator+(const T base) const
            {
                return { x + base, y + base, z + base };
            }

            Angle operator-(const T base) const
            {
                return { x - base, y - base, z - base };
            }

            Angle operator*(const T base) const
            {
                return { x * base, y * base, z * base };
            }

            Angle operator/(const T base) const
            {
                return { x / base, y / base, z / base };
            }

            bool is_valid() const
            {
                static_assert(std::is_floating_point_v<T> || std::is_integral_v<T>, "Only support floating point or integra");
                if constexpr (std::is_floating_point_v<T>)
                    return std::isfinite(x) && std::isfinite(y) && std::isfinite(z);

                return true;
            }

            bool is_zero() const
            {
                static_assert(std::is_floating_point_v<T> || std::is_integral_v<T>, "Only support floating point or integra");
                if constexpr (std::is_floating_point_v<T>)
                    return std::fpclassify(x) == 0 && std::fpclassify(y) == 0;
                return x == 0 && y == 0;
            }

            constexpr T length2d_sqr() const
            {
                static_assert(std::is_floating_point_v<T> || std::is_integral_v<T>, "Only support floating point or integra");

                return x * x + y * y;
            }

            constexpr T length2d() const
            {
                static_assert(std::is_floating_point_v<T> || std::is_integral_v<T>, "Only support floating point or integra");

                return sqrt(length2d_sqr());
            }

            Angle normalize()
            {
                x = std::isfinite(x) ? std::remainder(x, 360.f) : 0.f;
                y = std::isfinite(y) ? std::remainder(y, 360.f) : 0.f;
                z = 0.f;
                return *this;
            }

            Angle clamp()
            {
                x = std::clamp(x, -89, 89);
                y = std::clamp(y, -180, 180);
                z = std::clamp(z, -50, 50);
                return *this;
            }

            T fov(const Angle& angle) const
            {
                auto delta = (*this - angle).normalize();
                return std::hypot(delta.x, delta.y);
            }
        };
    }

    using fAngle = impl::Angle<float>;
}
