#pragma once

#include "Logging.h"

#define XSTR(x) STR(x)
#define STR(x) #x

#if APEX_ENABLE_ASSERTS >= 2

// Checks : Level 2 assertions : These are enabled only in Debug builds
#define axCheck(condition)						do { if (!(condition)) { axError("Check Failed! : " XSTR(condition)); DEBUG_BREAK(); } else {} } while (false)
#define axCheckFmt(condition, fmt, ...)			do { if (!(condition)) { axError("Check Failed! : " XSTR(condition)); axErrorFmt(fmt, ##__VA_ARGS__); DEBUG_BREAK(); } else {} } while (false)

// Asserts : Level 1 assertions : These are enabled in Debug and Development builds
#define axAssert(condition)						do { if (!(condition)) { axError("Assertion Failed! : " XSTR(condition)); DEBUG_BREAK(); } else {} } while (false)
#define axAssertFmt(condition, fmt, ...)		do { if (!(condition)) { axError("Assertion Failed! : " XSTR(condition)); axErrorFmt(fmt, ##__VA_ARGS__); DEBUG_BREAK(); } else {} } while (false)


// Strong Asserts : Level 0 assertions : These are enabled in all builds, and cause SIGKILL in Release builds
#define axStrongAssert(condition)				do { if (!(condition)) { axError("Strong Assertion Failed : " XSTR(condition)); DEBUG_BREAK(); } else {} } while (false)
#define axStrongAssertFmt(condition, fmt, ...)	do { if (!(condition)) { axError("Strong Assertion Failed : " XSTR(condition)); axErrorFmt(fmt, ##__VA_ARGS__); DEBUG_BREAK(); } else {} } while (false)

// Verifies : Level 0 assertions : These are enabled in all builds, but do not raise errors in Release builds
#define axVerify(condition)						((condition) || axVerifyFailedError("Verify Failed! : " XSTR(condition)))
#define axVerifyFmt(condition, fmt, ...)		((condition) || axVerifyFailedErrorFmt("Verify Failed! : " XSTR(condition) "\n" fmt, ##__VA_ARGS__))

#elif APEX_ENABLE_ASSERTS >= 1

// Checks : Level 2 assertions : These are enabled only in Debug builds
#define axCheck(condition)
#define axCheckFmt(condition, msg)

// Asserts : Level 1 assertions : These are enabled in Debug and Development builds
#define axAssert(condition) do { if (!(condition)) { axError("Assertion Failed! : " XSTR(condition)); DEBUG_BREAK(); } else {} } while (false)
#define axAssertFmt(condition, fmt, ...) do { if (!(condition)) { axError("Assertion Failed! : " XSTR(condition)); axErrorFmt(fmt, ##__VA_ARGS__); DEBUG_BREAK(); } else {} } while (false)

// Strong Asserts : Level 0 assertions : These are enabled in all builds, and cause SIGKILL in Release builds
#define axStrongAssert(condition) do { if (!(condition)) { axError("Strong Assertion Failed : " XSTR(condition)); DEBUG_BREAK(); } else {} } while (false)
#define axStrongAssertFmt(condition, fmt, ...) do { if (!(condition)) { axError("Strong Assertion Failed : " XSTR(condition)); axErrorFmt(fmt, ##__VA_ARGS__); DEBUG_BREAK(); } else {} } while (false)

// Verifies : Level 0 assertions : These are enabled in all builds, but do not raise errors in Release builds
#define axVerify(condition) do { if (!(condition)) { axError("Assertion Failed! : " XSTR(condition)); DEBUG_BREAK(); } else {} } while (false)
#define axVerifyFmt(condition, fmt, ...) do { if (!(condition)) { axError("Verify Failed! : " XSTR(condition) ""); axErrorFmt(fmt, ##__VA_ARGS__); DEBUG_BREAK(); } else {} } while (false)

#else

// Checks : Level 2 assertions : These are enabled only in Debug builds
#define axCheck(condition)
#define axCheckFmt(condition, msg)

// Asserts : Level 1 assertions : These are enabled in Debug and Development builds
#define axAssert(condition)
#define axAssertFmt(condition, msg)

// Strong Asserts : Level 0 assertions : These are enabled in all builds, and cause SIGKILL in Release builds
#define axStrongAssert(condition) do { if (!(condition)) { KILL(); } else {} } while (false)
#define axStrongAssertFmt(condition, msg) do { if (!(condition)) { KILL(); } else {} } while (false)

// Verifies : Level 0 assertions : These are enabled in all builds, but do not raise errors in Release builds
#define axVerify(condition) do { (condition); } while (false)
#define axVerifyFmt(condition, msg) do { (condition); } while (false)

#endif

#define TODO(...) axErrorFmt("TODO : " ##__VA_ARGS__)
