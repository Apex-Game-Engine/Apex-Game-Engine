#pragma once

#include "axLogging.h"

#define XSTR(x) STR(x)
#define STR(x) #x

#if APEX_ENABLE_ASSERTS >= 2

#define axCheck(condition) do { if (!(condition)) { axError("Assertion Failed! : " XSTR(condition)); __debugbreak(); } else {} } while (false)
#define axCheckMsg(condition, msg) do { if (!(condition)) { axError("Assertion Failed! : " XSTR(condition)); axError(msg); __debugbreak(); } else {} } while (false)

#define axAssert(condition) do { if (!(condition)) { axError("Assertion Failed! : " XSTR(condition)); __debugbreak(); } else {} } while (false)
#define axAssertMsg(condition, msg) do { if (!(condition)) { axError("Assertion Failed! : " XSTR(condition)); axError(msg); __debugbreak(); } else {} } while (false)

#define axVerify(condition) do { if (!(condition)) { axError("Assertion Failed! : " XSTR(condition)); __debugbreak(); } else {} } while (false)
#define axVerifyMsg(condition, msg) do { if (!(condition)) { axError("Assertion Failed! : " XSTR(condition) ""); axError(msg); __debugbreak(); } else {} } while (false)

#elif APEX_ENABLE_ASSERTS >= 1

#define axCheck(condition)
#define axCheck(condition, msg)

#define axAssert(condition) do { if (!(condition)) { axError("Assertion Failed! : " XSTR(condition)); __debugbreak(); } else {} } while (false)
#define axAssertMsg(condition, msg) do { if (!(condition)) { axError("Assertion Failed! : " XSTR(condition)); axError(msg); __debugbreak(); } else {} } while (false)

#define axVerify(condition) do { if (!(condition)) { axError("Assertion Failed! : " XSTR(condition)); __debugbreak(); } else {} } while (false)
#define axVerifyMsg(condition, msg) do { if (!(condition)) { axError("Assertion Failed! : " XSTR(condition)); axError(msg); __debugbreak(); } else {} } while (false)

#else

#define axCheck(condition)
#define axCheckMsg(condition, msg)

#define axAssert(condition)
#define axAssertMsg(condition, msg)

#define axVerify(condition) do { (condition); } while (false)
#define axVerifyMsg(condition, msg) do { (condition); } while (false)

#endif
