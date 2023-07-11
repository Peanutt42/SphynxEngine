#pragma once

#if defined(DEBUG) || defined(RELEASE)

#define TRACY_ENABLE
#include <tracy/Tracy.hpp>

#define SE_PROFILE_FRAME_END(name) FrameMarkNamed(name)

#define SE_PROFILE_FUNCTION() ZoneScoped
#define SE_PROFILE_SCOPE(name) ZoneScopedN(name)

#else

#define SE_PROFILE_FRAME_END(name)

#define SE_PROFILE_FUNCTION()
#define SE_PROFILE_SCOPE(name)

#endif