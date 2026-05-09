#pragma once

#include <array>

namespace arisu
{
    namespace impl
    {
        template <std::size_t rows, std::size_t cols>
        class Matrix
        {
        public:
            Matrix()
                : data()
            {
                static_assert(rows > 0 && cols > 0, "a matrix is at least 1x1");
            }

            Matrix(const std::array<float, rows * cols>& data)
                : data(data)
            {
                static_assert(rows > 0 && cols > 0, "a matrix is at least 1x1");
            }

            float* operator[](const int nIndex)
            {
                return data[nIndex];
            }

            const float* operator[](const int nIndex) const
            {
                return data[nIndex];
            }

            float& at(std::size_t row, std::size_t col)
            {
                return data[row][col];
            }

            [[nodiscard]] const float& at(std::size_t row, std::size_t col) const
            {
                return data[row][col];
            }

            [[nodiscard]] fVector3 at(std::size_t col) const
            {
                return { at(0, col), at(1, col), at(2, col) };
            }

            float* base()
            {
                return &data[0][0];
            }

            [[nodiscard]] const float* base() const
            {
                return &data[0][0];
            }

            float data[rows][cols];
        };
    }

    using Matrix3x4 = impl::Matrix<3, 4>;
    using Matrix4x4 = impl::Matrix<4, 4>;
}
