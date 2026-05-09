#pragma once

#include <cstdint>
#include <type_traits>

namespace arisu::impl
{
    template <typename Type = std::uint8_t>
    struct Color
    {
        Type r {}, g {}, b {}, a {};

        Color() = default;

        template <typename T = Type, std::enable_if_t<std::is_integral_v<T>, T> = 0>
        constexpr Color(T r, T g, T b, T a = 255) : r { Type(r) }, g { Type(g) }, b { Type(b) }, a { Type(a) }
        {
        }

        template <typename T = Type, std::enable_if_t<std::is_floating_point_v<T>, T> = 0>
        constexpr Color(T r, T g, T b, T a = 1.0f) : r { Type(r) }, g { Type(g) }, b { Type(b) }, a { Type(a) }
        {
        }

        constexpr Color(const Type* arr) : r { arr[0] }, g { arr[1] }, b { arr[2] }, a { arr[3] }
        {
        }

        template <typename T = Type, std::enable_if_t<std::is_integral_v<T>, T> = 0>
        [[nodiscard]] constexpr std::uint32_t to_hex() const
        {
            return ((r & 0xff) << 24) + ((g & 0xff) << 16) + ((b & 0xff) << 8) + (a & 0xff);
        }

        template <typename T = Type, std::enable_if_t<std::is_integral_v<T>, T> = 0>
        [[nodiscard]] constexpr std::uint32_t to_rgba() const
        {
            return ((r & 0xff) << 24) + ((g & 0xff) << 16) + ((b & 0xff) << 8) + (a & 0xff);
        }

        template <typename T = Type, std::enable_if_t<std::is_integral_v<T>, T> = 0>
        [[nodiscard]] constexpr std::uint32_t to_abgr() const
        {
            return ((a & 0xff) << 24) + ((b & 0xff) << 16) + ((g & 0xff) << 8) + (r & 0xff);
        }

        template <typename T = Type, std::enable_if_t<std::is_integral_v<T>, T> = 0>
        [[nodiscard]] constexpr std::uint32_t to_argb() const
        {
            return (a << 24) | (r << 16) | (g << 8) | b;
        }

        [[nodiscard]] constexpr Type* data()
        {
            return &r;
            /*return { r, g, b, a };*/
        }
    };
}
