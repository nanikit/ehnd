#pragma once

#ifdef BUILD_DLL
#define EHND_EXPORT __declspec(dllexport)
#else
#define EHND_EXPORT __declspec(dllimport)
#endif
