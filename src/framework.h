#pragma once

#define WIN32_LEAN_AND_MEAN      
// Windows Header Files


#include <Windows.h>
#include "lib/logger.hpp"
#include "lib/module.hpp"
#include "lib/pattern.hpp"
#include "lib/memory.hpp"
#include "lib/bitflags.hpp"
#include "lib/vectors.hpp"
#include "lib/angle.hpp"
#include "lib/color.hpp"
#include "lib/matrix.hpp"
#include "lib/math_funcs.hpp"
#include "lib/rtti_helper.hpp"
#include "lib/fnv1a_hash.h"
#include "lib/xorstr.hpp"
using Bitflags = arisu::Bitflag32;
using Vector2D = arisu::fVector2;
using Vector3D = arisu::fVector3;
using Angle = arisu::fAngle;
using Matrix3x4 = arisu::Matrix3x4;
using Matrix4x4 = arisu::Matrix4x4;
using ViewMatrix = arisu::Matrix4x4;
using u8Color = arisu::impl::Color<>;
using i32Color = arisu::impl::Color<std::int32_t>;

#include "modules.hpp"
#include"sdk/prototype_engine_struct.hpp"
#include "sdk/interfaces.hpp"
