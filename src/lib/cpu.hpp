#pragma once
#include <cstdint>
#include <bitset>
#include <array>
#include <vector>

#if defined(_WIN32)
#include <intrin.h>
#elif defined(__clang__) || defined(__GNUG__)
#include <cpuid.h>
#endif

namespace arisu
{
    enum class CPUArch
    {
        NONE,
        SSE,
        AVX2
    };

    namespace impl
    {
        struct CPUInfo
        {
            CPUInfo()
            {
#if defined(_WIN32)
                // https://learn.microsoft.com/en-us/cpp/intrinsics/cpuid-cpuidex?redirectedfrom=MSDN&view=msvc-170

                std::array<int, 4> cpui{};
                std::bitset<32> f_1_ECX_{};
                std::bitset<32> f_1_EDX_{};
                std::bitset<32> f_7_EBX_{};
                std::vector<std::array<int, 4>> data_{};

                // Calling __cpuid with 0x0 as the function_id argument
                // gets the number of the highest valid function ID.
                __cpuid(cpui.data(), 0);
                const auto nIds_ = cpui[0];

                for (int i = 0; i <= nIds_; ++i)
                {
                    __cpuidex(cpui.data(), i, 0);
                    data_.push_back(cpui);
                }

                // load bitset with flags for function 0x00000001
                if (nIds_ >= 1)
                {
                    f_1_ECX_ = data_[1][2];
                    f_1_EDX_ = data_[1][3];
                }

                // load bitset with flags for function 0x00000007
                if (nIds_ >= 7)
                {
                    f_7_EBX_ = data_[7][1];
                }

                if (f_1_ECX_[28] && f_7_EBX_[5])
                {
                    _arch = CPUArch::AVX2;
                }
                else if (f_1_EDX_[26])
                {
                    _arch = CPUArch::SSE;
                }

#elif defined(__clang__) || defined(__GNUG__)
                if (__builtin_cpu_supports("avx2"))
                    _arch = CPUArch::AVX2;
                else if (__builtin_cpu_supports("sse2"))
                    _arch = CPUArch::SSE;
#endif
            }

            [[nodiscard]] CPUArch get_arch() const
            {
                return _arch;
            }

        private:
            CPUArch _arch{};
        };
    }

    inline impl::CPUInfo cpu{};
}