#pragma once

#include "axLogging.h"

#define XSTR(x) STR(x)
#define STR(x) #x

#if APEX_ENABLE_ASSERTS >= 2

// Checks : Level 2 assertions : These are enabled only in Debug builds
#define axCheck(condition) do { if (!(condition)) { axError("Assertion Failed! : " XSTR(condition)); DEBUG_BREAK(); } else {} } while (false)
#define axCheckMsg(condition, msg) do { if (!(condition)) { axError("Assertion Failed! : " XSTR(condition)); axError(msg); DEBUG_BREAK(); } else {} } while (false)

// Asserts : Level 1 assertions : These are enabled in Debug and Development builds
#define axAssert(condition) do { if (!(condition)) { axError("Assertion Failed! : " XSTR(condition)); DEBUG_BREAK(); } else {} } while (false)
#define axAssertMsg(condition, msg) do { if (!(condition)) { axError("Assertion Failed! : " XSTR(condition)); axError(msg); DEBUG_BREAK(); } else {} } while (false)

// Verifies : Level 0 assertions : These are enabled in all builds, but do not raise errors in Release builds
#define axVerify(condition) do { if (!(condition)) { axError("Assertion Failed! : " XSTR(condition)); DEBUG_BREAK(); } else {} } while (false)
#define axVerifyMsg(condition, msg) do { if (!(condition)) { axError("Assertion Failed! : " XSTR(condition) ""); axError(msg); DEBUG_BREAK(); } else {} } while (false)

#elif APEX_ENABLE_ASSERTS >= 1

#define axCheck(condition)
#define axCheck(condition, msg)

#define axAssert(condition) do { if (!(condition)) { axError("Assertion Failed! : " XSTR(condition)); DEBUG_BREAK(); } else {} } while (false)
#define axAssertMsg(condition, msg) do { if (!(condition)) { axError("Assertion Failed! : " XSTR(condition)); axError(msg); DEBUG_BREAK(); } else {} } while (false)

#define axVerify(condition) do { if (!(condition)) { axError("Assertion Failed! : " XSTR(condition)); DEBUG_BREAK(); } else {} } while (false)
#define axVerifyMsg(condition, msg) do { if (!(condition)) { axError("Assertion Failed! : " XSTR(condition)); axError(msg); DEBUG_BREAK(); } else {} } while (false)

#else

#define axCheck(condition)
#define axCheckMsg(condition, msg)

#define axAssert(condition)
#define axAssertMsg(condition, msg)

#define axVerify(condition) do { (condition); } while (false)
#define axVerifyMsg(condition, msg) do { (condition); } while (false)

#endif
