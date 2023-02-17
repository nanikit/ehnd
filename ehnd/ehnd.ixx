module;

#include "targetver.h"

#include <Windows.h>

#include "macro.h"

export module Ehnd;

export extern "C" {
  EHND_EXPORT void J2K_Initialize(void);
  EHND_EXPORT bool __stdcall J2K_InitializeEx(LPCSTR name, LPCSTR key);
  EHND_EXPORT void __stdcall J2K_FreeMem(void*);
  EHND_EXPORT void J2K_GetPriorDict(void);
  EHND_EXPORT void J2K_GetProperty(void);
  EHND_EXPORT void __stdcall J2K_ReloadUserDict(void);
  EHND_EXPORT void J2K_SetDelJPN(void);
  EHND_EXPORT void J2K_SetField(void);
  EHND_EXPORT void J2K_SetHnj2han(void);
  EHND_EXPORT void J2K_SetJWin(void);
  EHND_EXPORT void J2K_SetPriorDict(void);
  EHND_EXPORT void J2K_SetProperty(void);
  EHND_EXPORT void J2K_StopTranslation(void);
  EHND_EXPORT void J2K_Terminate(void);
  EHND_EXPORT void J2K_TranslateChat(void);
  EHND_EXPORT void J2K_TranslateFM(void);
  EHND_EXPORT void J2K_TranslateMM(void);
  EHND_EXPORT void J2K_TranslateMMEx(void);
  EHND_EXPORT char* __stdcall J2K_TranslateMMNT(int data0, LPCSTR szText);
  EHND_EXPORT wchar_t* __stdcall J2K_TranslateMMNTW(int data0, LPCWSTR szText);
  EHND_EXPORT void J2K_GetJ2KMainDir(void);
  EHND_EXPORT void* msvcrt_free(void* _Memory);
  EHND_EXPORT void* msvcrt_malloc(size_t _Size);
  EHND_EXPORT void* msvcrt_fopen(const char* path, const char* mode);
};
