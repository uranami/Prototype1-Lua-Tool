#pragma once
#include "lib/module.hpp"

namespace arisu::game
{
    void SetupModules();

    // Prototype 1 uses prototypeenginef.dll
    inline impl::Module prototypeengine {};
}
