#include "framework.h"
#include "modules.hpp"

void FindPatternCallback(std::string_view pattern, std::expected<arisu::Address, arisu::pattern::Status> status, std::uintptr_t baseAddress)
{
    if (!status.has_value() || status == arisu::pattern::NoResult || !status->is_valid())
    {
        arisu::logger.error("No result was found with pattern --> {}", pattern);
        return;
    }

    if (status == arisu::pattern::InvalidPattern)
    {
        arisu::logger.error("Invalid pattern found --> {}", pattern);
        return;
    }

    arisu::logger.info("{} --> {:#x}(+{:#x})", pattern, status.value().ptr, status.value().ptr - baseAddress);
}

void arisu::game::SetupModules()
{
    prototypeengine = impl::Module("prototypeenginef.dll", nullptr, FindPatternCallback);
}
