#pragma once

#if defined(DEBUG) || defined(DEVELOPMENT)

#define TRACY_ON_DEMAND 1
#include <tracy/Tracy.hpp>

#define SE_PROFILE_FRAME_END() FrameMark

#define SE_PROFILE_FUNCTION() ZoneScoped
#define SE_PROFILE_SCOPE(name) ZoneScopedN(name)

#else

#define SE_PROFILE_FRAME_END()

#define SE_PROFILE_FUNCTION()
#define SE_PROFILE_SCOPE(name)

#endif