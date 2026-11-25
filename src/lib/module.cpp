#include "address.hpp"
#include "module.hpp"

#include "logger.hpp"
#include "pattern.hpp"

#include <Windows.h>

arisu::impl::Module::Module(std::string_view str, const FunctionCallbackFn& func, const FindPatternCallbackFn& find_pattern_callback)
{
    get_module_nfo(str, func);
    _find_pattern_callback = find_pattern_callback;
}

void arisu::impl::Module::get_module_nfo(std::string_view mod, const FunctionCallbackFn& func)
{
    const auto handle = GetModuleHandleA(mod.empty() ? nullptr : mod.data());
    if (!handle)
        return;

    const auto dos_header = reinterpret_cast<PIMAGE_DOS_HEADER>(handle);

    if (dos_header->e_magic != IMAGE_DOS_SIGNATURE)
        return;

    const auto bytes = reinterpret_cast<std::uint8_t*>(handle);

    const auto nt_header = reinterpret_cast<PIMAGE_NT_HEADERS>(bytes + dos_header->e_lfanew);

    if (nt_header->Signature != IMAGE_NT_SIGNATURE)
        return;

    this->_handle      = handle;
    this->_baseAddress = reinterpret_cast<uintptr_t>(handle);
    this->_size        = nt_header->OptionalHeader.SizeOfImage;
    auto section       = IMAGE_FIRST_SECTION(nt_header);

    for (auto i = 0; i < nt_header->FileHeader.NumberOfSections; i++, section++)
    {
        const auto is_executable = (section->Characteristics & IMAGE_SCN_MEM_EXECUTE) != 0;

        if (const auto is_readable = (section->Characteristics & IMAGE_SCN_MEM_READ) != 0; is_executable && is_readable)
        {
            const auto start = this->_baseAddress + section->VirtualAddress;
            const auto size  = std::min(section->SizeOfRawData, section->Misc.VirtualSize);

            this->_segments.emplace_back(start, reinterpret_cast<std::uint8_t*>(start), size);
        }
    }

    if (func)
    {
        const std::vector data(bytes, bytes + _size); 
        func(data);
    }

    logger.success("{} --> {:#x}", mod, _baseAddress);
}

arisu::Address arisu::impl::Module::find_pattern(std::string_view pattern) const
{
    for (auto&& segment : _segments)
    {
        if (auto result = pattern::find(segment.data, pattern))
        {
            if (_find_pattern_callback)
                _find_pattern_callback(pattern, result, _baseAddress);
            if (result.has_value())
                return result.value();
        }
    }

    return {};
}

void* arisu::impl::Module::get_proc(const std::string_view proc_name) const
{
    if (!this->_handle)
        return nullptr;

    if (const auto address = GetProcAddress(static_cast<HMODULE>(_handle), proc_name.data()); address)
    {
        return static_cast<void*>(address);
    }

    return nullptr;
}
