#pragma once

#if defined(DEBUG) || defined(RELEASE)

#define TRACY_ENABLE
#define TRACY_ON_DEMAND
#include <tracy/Tracy.hpp>

#define SE_PROFILE_FRAME_END(name) FrameMarkNamed(name)

#define SE_PROFILE_FUNCTION() ZoneScoped
// Adds the name to the current function name
#define SE_PROFILE_SCOPE(name) ZoneNamedN(TracyConcat(___tracy_scoped_zone, TracyLine), TracyConcat(TracyConcat(TracyFunction, "::"), name), true)

#else

#define SE_PROFILE_FRAME_END(name)

#define SE_PROFILE_FUNCTION()
#define SE_PROFILE_SCOPE(name)

#endif