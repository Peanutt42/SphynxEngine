#pragma once

#if defined(DEBUG) || defined(RELEASE)

#include <optick.h>

#define SE_PROFILE_FRAME(name) OPTICK_FRAME(name)
#define SE_PROFILE_THREAD(name) OPTICK_THREAD(name)
#define SE_PROFILE_SHUTDOWN() OPTICK_SHUTDOWN()

#define SE_PROFILE_FUNCTION() OPTICK_EVENT()
#define SE_PROFILE_SCOPE(name) OPTICK_EVENT(name)

#else

#define SE_PROFILE_FRAME(name)
#define SE_PROFILE_THREAD(name)
#define SE_PROFILE_SHUTDOWN()

#define SE_PROFILE_FUNCTION()
#define SE_PROFILE_SCOPE(name)

#endif