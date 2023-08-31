#pragma once

#ifdef _MSC_VER
#define DLL_EXPORT __declspec(dllexport)
#define DLL_IMPORT __declspec(dllimport)
#elif defined(__has_attribute)
#if __has_attribute(visibility)
#define DLL_EXPORT __attribute__((visibility("default")))
#else
#define DLL_EXPORT
#endif
#define DLL_IMPORT
#else
#error "Unknown Compiler, please implement DLL_EXPORT/DLL_IMPORT!"
#endif

#ifdef SE_EXPORT
#define SE_API DLL_EXPORT
#else
#define SE_API DLL_IMPORT
#endif

#pragma warning(disable: 4251) // warning when using stl containers in a SE_API declared class/struct