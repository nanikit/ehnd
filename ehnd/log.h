#pragma once
#include <winnt.h>

enum class log_category {
  normal = 0,
  error = 10,
  detail = 20,
  time = 30,
  skip_layer = 40,
  user_dict = 50,
};

void WriteLog(log_category category, const wchar_t* format, ...);
void WriteTextLog(const wchar_t* format, ...);

void LogStartMsg();
void CheckLogSize();
void CheckConsoleLine();

bool CreateLogWin(HINSTANCE);
void SetLogText(LPCWSTR);
void SetLogText(LPCWSTR, COLORREF, COLORREF);
void ClearLog(void);
void ShowLogWin(bool bShow);
bool IsShownLogWin(void);
DWORD WINAPI LogThreadMain(LPVOID lpParam);
LRESULT CALLBACK LogProc(HWND, UINT, WPARAM, LPARAM);