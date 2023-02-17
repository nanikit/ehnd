module;
#include <Windows.h>

#include <Richedit.h>
#include <tchar.h>

module Log;

import std.core;
import std.filesystem;

import Constants;
import Config;

HWND hLogWin, hLogRes;
HANDLE hLogEvent;

void LogStartMsg() {
  wchar_t lpEztPath[MAX_PATH];
  wchar_t lpExePath[MAX_PATH];

  Log(LogCategory::kNormal, L"──── ━━\n");
  Log(LogCategory::kNormal, L"Ehnd :: 엔드 - VER. {} :: COMPILE AT {}, {}\n", EHND_VER,
      _T(__DATE__), _T(__TIME__));
  Log(LogCategory::kNormal, L"──── ━━ Ehnd -- sokcuri.neko.kr --\n");
  Log(LogCategory::kNormal, L"\n");
  Log(LogCategory::kNormal, L"- 제작자 : {}\n", L"소쿠릿");
  Log(LogCategory::kNormal, L"━━━━━━━━━───────────-＊\n");
  Log(LogCategory::kNormal, L"EzTransPath : {}\n", pConfig->GetEztransPath());
  Log(LogCategory::kNormal, L"ExecutePath : {}\n", pConfig->GetExecutablePath());
  return;
}

void CheckLogSize() {
  using namespace std;
  FILE* fp;

  // 로그 사용안할때 끄기
  if (!pConfig->GetFileLogSwitch()) return;

  auto path = pConfig->GetFileLogDirectory() + L"\\ehnd_log.log";

  if (_wfopen_s(&fp, path.c_str(), L"a+t,ccs=UTF-8")) return;

  // fpos = ftell(fp);
  fseek(fp, 0, SEEK_END);
  int fsize = ftell(fp);
  int cf_size = pConfig->GetFileLogSize();

  if (cf_size != 0 && cf_size * 1024 < fsize) {
    fclose(fp);
    filesystem::remove(path);
  } else {
    fclose(fp);
  }
}

void CheckConsoleLine() {
  int cf_line = pConfig->GetConsoleMaxLine();

  if (cf_line != 0 && logLine > cf_line) {
    ClearLog();
    logLine = 0;
  }
}

bool CreateLogWin(HINSTANCE hInst) {
  LoadLibrary(TEXT("Msftedit.dll"));
  WNDCLASSEX wc = {
    0,
  };
  wc.cbSize = sizeof(WNDCLASSEX);
  wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
  wc.hInstance = hInst;
  wc.lpfnWndProc = LogProc;
  wc.lpszClassName = TEXT("EhndLogWin");
  wc.style = CS_HREDRAW | CS_VREDRAW;

  RegisterClassEx(&wc);
  // if (!) MessageBox(0, L"log reg failed", 0, 0);

  SECURITY_ATTRIBUTES thAttr;
  thAttr.bInheritHandle = false;
  thAttr.lpSecurityDescriptor = NULL;
  thAttr.nLength = sizeof(SECURITY_ATTRIBUTES);

  hLogEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
  if (!hLogEvent) {
    Log(LogCategory::kError, L"CreateLogWin : Event Init Error");
    return 0;
  }
  HANDLE hThread = CreateThread(&thAttr, 0, LogThreadMain, hInst, 0, NULL);
  if (hThread == NULL) {
    Log(LogCategory::kError, L"CreateLogWin : Log Thread Create Error");
  }

  // 로그 윈도우가 초기화될때까지 기다림
  WaitForSingleObject(hLogEvent, INFINITE);
  return 0;
}

DWORD WINAPI LogThreadMain(LPVOID lpParam) {
  auto hInst = static_cast<HINSTANCE>(lpParam);
  wchar_t wszTitle[255];

  wsprintf(wszTitle, L"Ehnd Log Window :: VER. %s (%s, %s)", EHND_VER, _T(__DATE__), _T(__TIME__));

  hLogWin = CreateWindowEx(0, L"EhndLogWin", wszTitle, WS_OVERLAPPEDWINDOW, 64, 64, 640, 480, 0, 0,
                           hInst, 0);
  if (!hLogWin) {
    Log(LogCategory::kError, L"LogThreadMain : Log Window Create Failed");
    return 0;
  }

  hLogRes = CreateWindowEx(
    0, MSFTEDIT_CLASS, L"",
    WS_VISIBLE | WS_CHILD | WS_VSCROLL | ES_MULTILINE | ES_LEFT | ES_NOHIDESEL | ES_AUTOVSCROLL, 0,
    0, 640, 480, hLogWin, NULL, hInst, NULL);
  if (!hLogRes) {
    Log(LogCategory::kError, L"LogThreadMain : Log Edit Create Failed");
  }

  CHARFORMAT2 cf;
  cf.cbSize = sizeof(CHARFORMAT2);
  cf.dwMask = CFM_COLOR | CFM_BACKCOLOR | CFM_EFFECTS2 | CFM_FACE | CFM_SIZE;
  cf.crTextColor = RGB(0, 0, 0);
  cf.crBackColor = RGB(255, 255, 255);
  cf.yHeight = 12 * 20;

  wcscpy_s(cf.szFaceName, L"굴림");
  cf.dwEffects = CFE_BOLD;
  SendMessage(hLogRes, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);
  SendMessage(hLogRes, EM_REPLACESEL, TRUE, (LPARAM)L"");

  ShowWindow(hLogRes, true);

  MSG msg;
  BOOL bRet;

  // 쓰레드 초기화 완료
  SetEvent(hLogEvent);

  while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
  return msg.wParam;
}

void SetLogText(LPCWSTR Text) {
  SetLogText(Text, RGB(0, 0, 0), RGB(255, 255, 255));
}

void SetLogText(LPCWSTR Text, COLORREF crText, COLORREF crBackground) {
  CHARRANGE cr = {LONG_MAX, LONG_MAX};
  SendMessage(hLogRes, EM_EXSETSEL, 0, (LPARAM)&cr);

  CHARFORMAT2 cf;
  cf.cbSize = sizeof(CHARFORMAT2);
  cf.dwMask = CFM_COLOR | CFM_BACKCOLOR | CFM_EFFECTS2 | CFM_FACE | CFM_SIZE;
  cf.crTextColor = crText;
  cf.crBackColor = crBackground;
  cf.dwEffects = CFE_BOLD;

  int nLogFntSize = pConfig->GetConsoleFontSize();
  cf.yHeight = nLogFntSize * 20;
  wcscpy_s(cf.szFaceName, pConfig->GetConsoleFontName());
  SendMessage(hLogRes, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);

  SendMessage(hLogRes, EM_REPLACESEL, TRUE, (LPARAM)Text);
}
void ClearLog(void) {
  CHARRANGE cr = {0, LONG_MAX};
  SendMessage(hLogRes, EM_EXSETSEL, 0, (LPARAM)&cr);
  SendMessage(hLogRes, EM_REPLACESEL, TRUE, NULL);
}

void ShowLogWin(bool bShow) {
  if (!bShow) ClearLog();
  ShowWindow(hLogWin, bShow);
  ShowWindow(hLogRes, bShow);
}

bool IsShownLogWin(void) {
  return (GetWindowLong(hLogWin, GWL_STYLE) & WS_VISIBLE) ? true : false;
}

LRESULT CALLBACK LogProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam) {
  switch (Message) {
    case WM_SIZE:
      int w, h;
      w = LOWORD(lParam);
      h = HIWORD(lParam);
      MoveWindow(hLogRes, 0, 0, w, h, true);
      break;
    case WM_CLOSE:
      ShowLogWin(false);
      return 0;
  }
  return DefWindowProc(hWnd, Message, wParam, lParam);
}
