#pragma once

#if defined(DEBUG) || defined(DEVELOPMENT)

#define TRACY_ENABLE
#define TRACY_ON_DEMAND
#include <tracy/Tracy.hpp>

#define SE_PROFILE_FRAME_END(name) FrameMarkNamed(name)

#define SE_PROFILE_FUNCTION() ZoneScoped
// Adds the name to the current function name
#define SE_PROFILE_SCOPE(name) ZoneScopedN(name)

#else

#define SE_PROFILE_FRAME_END(name)

#define SE_PROFILE_FUNCTION()
#define SE_PROFILE_SCOPE(name)

#endif