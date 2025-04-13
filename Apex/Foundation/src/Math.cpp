#ifdef APEX_MATH_NOINLINE_IMPL

#define APEX_MATH_SKIP_INLINE_IMPL
#include "Math/Math.h"
#include "Math/Vector3.h"
#include "Math/Vector4.h"
#undef APEX_MATH_SKIP_INLINE_IMPL

#include "Math/Math.inl"
#include "Math/Vector.inl"

#endif
