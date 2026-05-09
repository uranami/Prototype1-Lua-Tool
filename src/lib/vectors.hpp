#pragma once

#include <cmath>
#include <type_traits>
#include <numbers>

namespace arisu
{
    namespace impl
    {
        template <typename T>
        class Angle;

        template <typename T>
        class Vector2
        {
          public:
            T x, y;

            Vector2(T x_ = 0, T y_ = 0)
             : x(x_),
               y(y_)
            {
            }

            Vector2(T* arr)
             : x(arr[0]),
               y(arr[1])
            {
            }

            constexpr Vector2& operator=(const Vector2& base)
            {
                x = base.x;
                y = base.y;
                return *this;
            }

            constexpr Vector2& operator+=(const Vector2& base)
            {
                x += base.x;
                y += base.y;
                return *this;
            }

            constexpr Vector2& operator-=(const Vector2& base)
            {
                x -= base.x;
                y -= base.y;
                return *this;
            }

            constexpr Vector2& operator*=(const Vector2& base)
            {
                x *= base.x;
                y *= base.y;
                return *this;
            }

            constexpr Vector2& operator/=(const Vector2& base)
            {
                x /= base.x;
                y /= base.y;
                return *this;
            }

            constexpr Vector2& operator+=(const T base)
            {
                x += base;
                y += base;
                return *this;
            }

            constexpr Vector2& operator-=(const T base)
            {
                x -= base;
                y -= base;
                return *this;
            }

            constexpr Vector2& operator*=(const T base)
            {
                x *= base;
                y *= base;
                return *this;
            }

            constexpr Vector2& operator/=(const T base)
            {
                x /= base;
                y /= base;
                return *this;
            }

            Vector2 operator+(const Vector2& base)
            {
                return { x + base.x, y + base.y };
            }

            Vector2 operator-(const Vector2& base)
            {
                return { x - base.x, y - base.y };
            }

            Vector2 operator*(const Vector2& base)
            {
                return { x * base.x, y * base.y };
            }

            Vector2 operator/(const Vector2& base)
            {
                return { x / base.x, y / base.y };
            }

            Vector2 operator+(const T base)
            {
                return { x + base, y + base };
            }

            Vector2 operator-(const T base)
            {
                return { x - base, y - base };
            }

            Vector2 operator*(const T base)
            {
                return { x * base, y * base };
            }

            Vector2 operator/(const T base)
            {
                return { x / base, y / base };
            }

            bool is_zero() const
            {
                static_assert(std::is_floating_point_v<T> || std::is_integral_v<T>, "Only support floating point or integra");
                if constexpr (std::is_floating_point_v<T>)
                    return std::fpclassify(x) == FP_ZERO && std::fpclassify(y) == FP_ZERO;
                return x == 0 && y == 0;
            }

            constexpr T length_sqr() const
            {
                return x * x + y + y;
            }

            constexpr T length() const
            {
                static_assert(std::is_floating_point_v<T> || std::is_integral_v<T>, "Only support floating point or integral type");

                return sqrt(length_sqr());
            }

            bool is_valid() const
            {
                static_assert(std::is_floating_point_v<T> || std::is_integral_v<T>, "Only support floating point or integral type");
                if constexpr (std::is_floating_point_v<T>)
                    return std::isfinite(x) && std::isfinite(y);

                return true;
            }

            T dist_to(const Vector2& end) const
            {
                return (*this - end).length();
            }

            T dist_to_sqr(const Vector2& end) const
            {
                return (*this - end).length_sqr();
            }

            T normalize_in_place()
            {
                static_assert(std::is_floating_point_v<T>, "This can only be used for floating point type");

                auto length = this->length();
                auto radius = 1.f / (length + std::numeric_limits<T>::epsilon());

                x *= radius;
                y *= radius;

                return length;
            }
        };

        template <typename T>
        class Vector3
        {
          public:
            T x, y, z;

            Vector3(T x_ = 0, T y_ = 0, T z_ = 0)
             : x(x_),
               y(y_),
               z(z_)
            {
            }

            Vector3(const T* arr)
             : x(arr[0]),
               y(arr[1]),
               z(arr[2])
            {
            }

            Vector3(const Vector2<T>& vec)
             : x(vec.x),
               y(vec.y),
               z(0)
            {
            }

            /*constexpr Vector3<T>& operator=(const Vector3<T>& base)
            {
                x = base.x;
                y = base.y;
                z = base.z;
                return *this;
            }*/

            constexpr Vector3<T>& operator+=(const Vector3<T>& base)
            {
                x += base.x;
                y += base.y;
                z += base.z;
                return *this;
            }

            constexpr Vector3<T>& operator-=(const Vector3<T>& base)
            {
                x -= base.x;
                y -= base.y;
                z -= base.z;
                return *this;
            }

            constexpr Vector3<T>& operator*=(const Vector3<T>& base)
            {
                x *= base.x;
                y *= base.y;
                z *= base.z;
                return *this;
            }

            constexpr Vector3<T>& operator/=(const Vector3<T>& base)
            {
                x /= base.x;
                y /= base.y;
                z /= base.z;
                return *this;
            }

            constexpr Vector3<T>& operator+=(const T base)
            {
                x += base;
                y += base;
                z += base;
                return *this;
            }

            constexpr Vector3<T>& operator-=(const T base)
            {
                x -= base;
                y -= base;
                z -= base;
                return *this;
            }

            constexpr Vector3<T>& operator*=(const T base)
            {
                x *= base;
                y *= base;
                z *= base;
                return *this;
            }

            constexpr Vector3<T>& operator/=(const T base)
            {
                x /= base;
                y /= base;
                z /= base;
                return *this;
            }

            Vector3<T> operator+(const Vector3<T>& base) const
            {
                return { x + base.x, y + base.y, z + base.z };
            }

            Vector3<T> operator-(const Vector3<T>& base) const
            {
                return { x - base.x, y - base.y, z - base.z };
            }

            Vector3<T> operator*(const Vector3<T>& base) const
            {
                return { x * base.x, y * base.y, z * base.z };
            }

            Vector3<T> operator/(const Vector3<T>& base) const
            {
                return { x / base.x, y / base.y, z / base.z };
            }

            Vector3<T> operator+(const T base) const
            {
                return { x + base, y + base, z + base };
            }

            Vector3<T> operator-(const T base) const
            {
                return { x - base, y - base, z - base };
            }

            Vector3<T> operator*(const T base) const
            {
                return { x * base, y * base, z * base };
            }

            Vector3<T> operator/(const T base) const
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

            constexpr T length3d_sqr() const
            {
                static_assert(std::is_floating_point_v<T> || std::is_integral_v<T>, "Only support floating point or integra");

                return x * x + y + y + z * z;
            }

            constexpr T length3d() const
            {
                static_assert(std::is_floating_point_v<T> || std::is_integral_v<T>, "Only support floating point or integra");

                return sqrt(length3d_sqr());
            }

            T dist_to_3d(const Vector3<T>& end) const
            {
                return (*this - end).length3d();
            }

            T dist_to_3d_sqr(const Vector3<T>& end) const
            {
                return (*this - end).length3d();
            }

            T dist_to_2d(const Vector3<T>& end) const
            {
                return (*this - end).length2d();
            }

            T dist_to_2d_sqr(const Vector3<T>& end) const
            {
                return (*this - end).length2d_sqr();
            }

            Vector3<T> normalized() const
            {
                auto out = *this;
                out.normalize_in_place();
                return out;
            }

            T normalize_in_place()
            {
                static_assert(std::is_floating_point_v<T>, "This can only be used for floating point type");

                auto length = this->length3d();
                auto radius = 1.f / (length + std::numeric_limits<T>::epsilon());

                x *= radius;
                y *= radius;
                z *= radius;

                return length;
            }

            T yaw_angle() const
            {
                if (std::fpclassify(y) == 0)
                    return 0;

                return std::atan2(y, x) * 180.f / static_cast<float>(std::numbers::pi);
            }

            T pitch_angle() const
            {
                if (std::fpclassify(x) == 0.f)
                    return z > 0.f ? 270.f : 90.f;

                return std::atan2(z, length2d()) * -180.f / static_cast<float>(std::numbers::pi);
            }

            T dot_product(const Vector3& vec) const
            {
                return x * vec.x + y * vec.y + z * vec.z;
            }

            impl::Angle<T> to_angle()
            {
                return { pitch_angle(), yaw_angle(), 0.f };
            }
        };
    }

    using fVector2 = impl::Vector2<float>;
    using fVector3 = impl::Vector3<float>;
}
