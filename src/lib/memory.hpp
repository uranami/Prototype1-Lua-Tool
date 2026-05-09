#pragma once

#include "address.hpp"
#include <memory>

namespace arisu
{
    class Memory
    {
    public:
        template <size_t Index>
        static Address get_vfunc_address(Address vtable)
        {
            return vtable.deref().cast<uintptr_t*>()[Index];
        }

        static Address get_vfunc_address(Address vtable, std::size_t index)
        {
            return vtable.deref().cast<uintptr_t*>()[index];
        }

        #ifdef _WIN32
        template <typename T, size_t Index, typename... Args>
        static T virtual_call(void* vtable, Args... args)
        {
            using fn = T(__thiscall*)(void*, decltype(args)...);
            return get_vfunc_address<Index>(vtable).template cast<fn>()(vtable, args...);
        }

        template <typename T, size_t Index, typename... Args>
        static T virtual_ccall(void* vtable, Args... args)
        {
            using fn = T(__cdecl*)(void*, decltype(args)...);
            return get_vfunc_address<Index>(vtable).template cast<fn>()(vtable, args...);
        }
        #elif LINUX
        template <typename T, size_t Index, typename... Args>
        static constexpr T virtual_call(void* vtable, Args... args)
        {
            using fn = T (*)(void*, decltype(args)...);
            return get_vfunc_address<Index>(vtable).template cast<fn>()(vtable, args...);
        }
        #endif
    };
}
