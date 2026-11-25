#pragma once

#include <cstdint>
#include <utility>

namespace arisu
{
    namespace impl
    {
        template <typename T = std::uintptr_t>
        struct BitflagBase
        {
          private:
            T _flags;

          public:
            explicit BitflagBase(T flags = T {}) : _flags(flags)
            {
            }

            BitflagBase(const BitflagBase& other) : _flags(other._flags)
            {
            }

            BitflagBase(BitflagBase&& other) noexcept : _flags(std::move(other._flags))
            {
                other._flags = T {};
            }

            BitflagBase& operator=(T v)
            {
                _flags = v;
                return *this;
            }

            bool has_flag(T flag)
            {
                return _flags & flag;
            }

            void add_flag(T flag)
            {
                _flags |= flag;
            }

            void remove_flag(T flag)
            {
                _flags &= ~flag;
            }

            bool empty() const
            {
                return !_flags;
            }

            T raw() const
            {
                return _flags;
            }

            BitflagBase operator&(const BitflagBase& other) const
            {
                return BitflagBase(_flags & other._flags);
            }

            BitflagBase operator|(const BitflagBase& other) const
            {
                return BitflagBase(_flags | other._flags);
            }

            BitflagBase operator^(const BitflagBase& other) const
            {
                return BitflagBase(_flags ^ other._flags);
            }

            BitflagBase operator~() const
            {
                return BitflagBase(~_flags);
            }

            BitflagBase& operator&=(const BitflagBase& other)
            {
                _flags &= other._flags;
                return *this;
            }

            BitflagBase& operator|=(const BitflagBase& other)
            {
                _flags |= other._flags;
                return *this;
            }

            BitflagBase& operator^=(const BitflagBase& other)
            {
                _flags ^= other._flags;
                return *this;
            }
        };
    }

    using Bitflag   = impl::BitflagBase<std::uintptr_t>;
    using Bitflag32 = impl::BitflagBase<std::uint32_t>;
    using Bitflag64 = impl::BitflagBase<std::uint64_t>;
}
