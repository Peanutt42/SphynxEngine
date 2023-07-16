#pragma once

#ifdef SE_EXPORT
#define SE_API __declspec(dllexport)
#else
#define SE_API __declspec(dllimport)
#endif

#pragma warning(disable: 4251) // warning when using stl containers in a SE_API declared class/struct