#pragma once
#include <winnt.h>

#define NORMAL_LOG 0
#define ERROR_LOG 10
#define DETAIL_LOG 20
#define TIME_LOG 30
#define SKIPLAYER_LOG 40
#define USERDIC_LOG 50

void WriteLog(int LogType, const wchar_t* format, ...);
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