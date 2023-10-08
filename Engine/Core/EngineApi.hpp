#pragma once

#if defined(_MSC_VER)
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT
#endif

#ifdef SE_EXPORT
#define SE_API DLL_EXPORT
#else
#define SE_API
#endif