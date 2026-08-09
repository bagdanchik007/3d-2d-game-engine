#pragma once

// Convenience aggregate for call sites using several math types together
// (e.g. a Transform component needing Vec3 + Quaternion + Mat4). Individual
// headers remain independently includable for translation units that only
// need one piece - this file trades a few extra parsed lines for fewer
// #include lines at the call site, nothing more.
#include "Engine/Math/Mat4.h"
#include "Engine/Math/MathUtils.h"
#include "Engine/Math/Quaternion.h"
#include "Engine/Math/Vec2.h"
#include "Engine/Math/Vec3.h"
#include "Engine/Math/Vec4.h"
