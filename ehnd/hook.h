#pragma once

#include <Windows.h>

#include <optional>
#include <string>
#include <string_view>

#include "macro.h"

bool hook();
bool hook_userdict();
bool hook_userdict2();
bool hook_getwordinfo();
void user_wordinfo();

bool GetRealWC2MB();
bool GetRealMB2WC();
void* fopen_patch(const char* path, const char* mode);
void userdict_patch();
void userdict_log(char* s);
void userdict_log2(int idx, int num);
bool userdict_check();
int userdict_proc(char* word_str, char* base, int cur, int total);

// 아랄트랜스 후킹 시 문제 발생 방지 기능 포함
int __stdcall WideCharToMultiByteWithAral(
  _In_ UINT CodePage, _In_ DWORD dwFlags, _In_NLS_string_(cchWideChar) LPCWCH lpWideCharStr,
  _In_ int cchWideChar, _Out_writes_bytes_to_opt_(cbMultiByte, return) LPSTR lpMultiByteStr,
  _In_ int cbMultiByte, _In_opt_ LPCCH lpDefaultChar, _Out_opt_ LPBOOL lpUsedDefaultChar);

// 아랄트랜스 후킹 시 문제 발생 방지 기능 포함
int __stdcall MultiByteToWideCharWithAral(_In_ UINT CodePage, _In_ DWORD dwFlags,
                                          _In_NLS_string_(cbMultiByte) LPCCH lpMultiByteStr,
                                          _In_ int cbMultiByte,
                                          _Out_writes_to_opt_(cchWideChar, return)
                                            LPWSTR lpWideCharStr,
                                          _In_ int cchWideChar);

std::wstring MultiByteToWide(std::string_view source, UINT codePage, bool useOriginal = false,
                             const std::optional<std::wstring>& buffer = std::nullopt);

extern LPBYTE lpfnRetn;
extern LPBYTE lpfnfopen;
extern LPBYTE lpfnwc2mb;
extern LPBYTE lpfnmb2wc;
extern int wc2mb_type;
extern int mb2wc_type;

extern FARPROC apfnEzt[100];
extern FARPROC apfnMsv[100];

extern std::string dic_path;
