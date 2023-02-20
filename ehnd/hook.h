#pragma once

#include <Windows.h>

#include <Psapi.h>

#include <optional>
#include <string>

extern std::string dic_path;

extern FARPROC apfnEzt[100];
extern HMODULE hEzt, hMsv;

extern bool(__stdcall* j2k_initialize_ex)(LPCSTR name, LPCSTR key);
extern char*(__stdcall* j2k_translate_mmnt)(int data0, LPCSTR szText);
extern void* (*msvcrt_fopen_ptr)(const char* path, const char* mode);
extern void* (*msvcrt_free_ptr)(void* _Memory);
extern void* (*msvcrt_malloc_ptr)(size_t _Size);

extern void (*user_dictionary_conversion_logger)(int idx, int num);

bool hook();
bool hook_userdict(void);
bool hook_userdict2(void);
bool hook_getwordinfo(void);

bool GetRealWC2MB(void);
bool GetRealMB2WC(void);

#pragma warning(push)
#pragma warning(disable : 6054)
int __stdcall WideCharToMultiByteWithAral(
  _In_ UINT CodePage, _In_ DWORD dwFlags, _In_NLS_string_(cchWideChar) LPCWCH lpWideCharStr,
  _In_ int cchWideChar, _Out_writes_bytes_to_opt_(cbMultiByte, return) LPSTR lpMultiByteStr,
  _In_ int cbMultiByte, _In_opt_ LPCCH lpDefaultChar, _Out_opt_ LPBOOL lpUsedDefaultChar);

int __stdcall MultiByteToWideCharWithAral(_In_ UINT CodePage, _In_ DWORD dwFlags,
                                          _In_NLS_string_(cbMultiByte) LPCCH lpMultiByteStr,
                                          _In_ int cbMultiByte,
                                          _Out_writes_to_opt_(cchWideChar, return)
                                            LPWSTR lpWideCharStr,
                                          _In_ int cchWideChar);
#pragma warning(pop)

std::wstring MultiByteToWide(std::string_view source, UINT codePage, bool useOriginal = false,
                             const std::optional<std::wstring>& buffer = std::nullopt);
