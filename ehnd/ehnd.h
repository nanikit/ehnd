// 다음 ifdef 블록은 DLL에서 내보내기하는 작업을 쉽게 해 주는 매크로를 만드는
// 표준 방식입니다. 이 DLL에 들어 있는 파일은 모두 명령줄에 정의된 _EXPORTS 기호로
// 컴파일되며, 다른 프로젝트에서는 이 기호를 정의할 수 없습니다.
// 이렇게 하면 소스 파일에 이 파일이 들어 있는 다른 모든 프로젝트에서는
// EHND_API 함수를 DLL에서 가져오는 것으로 보고, 이 DLL은
// 이 DLL은 해당 매크로로 정의된 기호가 내보내지는 것으로 봅니다.

#pragma once
#include <Windows.h>
#include <tchar.h>

constexpr auto EHND_VER = _T("V3.20");

#define EHND_EXPORT __declspec(dllexport)

extern "C" {
EHND_EXPORT void J2K_Initialize(void);
EHND_EXPORT void __stdcall J2K_InitializeEx(LPCSTR name, LPCSTR key);
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
EHND_EXPORT void* __stdcall J2K_TranslateMMNT(int data0, LPCSTR szText);
EHND_EXPORT wchar_t* __stdcall J2K_TranslateMMNTW(int data0, LPCWSTR szText);
EHND_EXPORT void J2K_GetJ2KMainDir(void);
EHND_EXPORT void* msvcrt_free(void* _Memory);
EHND_EXPORT void* msvcrt_malloc(size_t _Size);
EHND_EXPORT void* msvcrt_fopen(char* path, char* mode);
};

bool GetLoadPath(LPWSTR Path, int Size);
bool GetExecutePath(LPWSTR Path, int Size);

extern FARPROC apfnEzt[100];
extern FARPROC apfnMsv[100];
