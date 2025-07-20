#pragma once

#include "Logging.h"
#include "Macros.h"

#if APEX_ENABLE_ASSERTS >= 2

// Checks : Level 2 assertions : These are enabled only in Debug builds
#define axCheck(condition)						do { if (!(condition)) { axError("Check Failed! : " STR(condition)); DEBUG_BREAK(); } else {} } while (false)
#define axCheckFmt(condition, fmt, ...)			do { if (!(condition)) { axError("Check Failed! : " STR(condition)); axErrorFmt(fmt, ##__VA_ARGS__); DEBUG_BREAK(); } else {} } while (false)

// Asserts : Level 1 assertions : These are enabled in Debug and Development builds
#define axAssert(condition)						do { if (!(condition)) { axError("Assertion Failed! : " STR(condition)); DEBUG_BREAK(); } else {} } while (false)
#define axAssertFmt(condition, fmt, ...)		do { if (!(condition)) { axError("Assertion Failed! : " STR(condition)); axErrorFmt(fmt, ##__VA_ARGS__); DEBUG_BREAK(); } else {} } while (false)


// Strong Asserts : Level 0 assertions : These are enabled in all builds, and cause SIGKILL in Release builds
#define axStrongAssert(condition)				do { if (!(condition)) { axError("Strong Assertion Failed : " STR(condition)); DEBUG_BREAK(); } else {} } while (false)
#define axStrongAssertFmt(condition, fmt, ...)	do { if (!(condition)) { axError("Strong Assertion Failed : " STR(condition)); axErrorFmt(fmt, ##__VA_ARGS__); DEBUG_BREAK(); } else {} } while (false)

// Verifies : Level 0 assertions : These are enabled in all builds, but do not raise errors in Release builds
#define axVerify(condition)						((condition) || ((axVerifyFailedError("Verify Failed! : " STR(condition)) || DEBUG_BREAK())) && false)
#define axVerifyFmt(condition, fmt, ...)		((condition) || ((axVerifyFailedErrorFmt("Verify Failed! : " STR(condition) "\n" fmt, ##__VA_ARGS__) || DEBUG_BREAK())) && false)

#elif APEX_ENABLE_ASSERTS >= 1

// Checks : Level 2 assertions : These are enabled only in Debug builds
#define axCheck(condition)
#define axCheckFmt(condition, msg)

// Asserts : Level 1 assertions : These are enabled in Debug and Development builds
#define axAssert(condition)						do { if (!(condition)) { axError("Assertion Failed! : " STR(condition)); DEBUG_BREAK(); } else {} } while (false)
#define axAssertFmt(condition, fmt, ...)		do { if (!(condition)) { axError("Assertion Failed! : " STR(condition)); axErrorFmt(fmt, ##__VA_ARGS__); DEBUG_BREAK(); } else {} } while (false)

// Strong Asserts : Level 0 assertions : These are enabled in all builds, and cause SIGKILL in Release builds
#define axStrongAssert(condition)				do { if (!(condition)) { axError("Strong Assertion Failed : " STR(condition)); DEBUG_BREAK(); } else {} } while (false)
#define axStrongAssertFmt(condition, fmt, ...)	do { if (!(condition)) { axError("Strong Assertion Failed : " STR(condition)); axErrorFmt(fmt, ##__VA_ARGS__); DEBUG_BREAK(); } else {} } while (false)

// Verifies : Level 0 assertions : These are enabled in all builds, but do not raise errors in Release builds
#define axVerify(condition)						((condition) || axVerifyFailedError("Verify Failed! : " STR(condition)))
#define axVerifyFmt(condition, fmt, ...)		((condition) || axVerifyFailedErrorFmt("Verify Failed! : " STR(condition) "\n" fmt, ##__VA_ARGS__))

#else

// Checks : Level 2 assertions : These are enabled only in Debug builds
#define axCheck(condition)
#define axCheckFmt(condition, msg)

// Asserts : Level 1 assertions : These are enabled in Debug and Development builds
#define axAssert(condition)
#define axAssertFmt(condition, msg)

// Strong Asserts : Level 0 assertions : These are enabled in all builds, and cause SIGKILL in Release builds
#define axStrongAssert(condition)				do { if (!(condition)) { KILL(); } else {} } while (false)
#define axStrongAssertFmt(condition, msg)		do { if (!(condition)) { KILL(); } else {} } while (false)

// Verifies : Level 0 assertions : These are enabled in all builds, but do not raise errors in Release builds
#define axVerify(condition)						(condition)
#define axVerifyFmt(condition, msg)				(condition)

#endif

#define TODO(...) axErrorFmt("TODO : " ##__VA_ARGS__)
