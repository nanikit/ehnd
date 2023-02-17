#pragma once

#include <Windows.h>

#include <array>
#include <chrono>
#include <format>
#include <fstream>

#include "config.h"

enum class LogCategory {
  kNormal = 0,
  kError = 10,
  kDetail = 20,
  kTime = 30,
  kSkipLayer = 40,
  kUserDict = 50,
};

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

extern Config* pConfig;
extern int logLine;

template <typename... Args>
auto Log(LogCategory category, const std::wformat_string<Args...> fmt, Args&&... args) {
  using namespace std;
  static auto zone = chrono::current_zone();

  if (!pConfig->GetLogTime() && category == LogCategory::kTime) return;
  if (!pConfig->GetLogDetail() && category == LogCategory::kDetail) return;
  if (!pConfig->GetLogSkipLayer() && category == LogCategory::kSkipLayer) return;
  if (!pConfig->GetLogUserDic() && category == LogCategory::kUserDict) return;

  bool doWindowLog = IsShownLogWin();
  bool doFileLog = pConfig->GetFileLogSwitch();

  if (doWindowLog || doFileLog) {
    auto message = format(fmt, std::forward<Args&&>(args)...);

    if (doWindowLog) SetLogText(message.c_str());
    if (doFileLog) {
      array<wchar_t, MAX_PATH> lpFileName;
      if (pConfig->GetFileLogEztLoc())
        GetLoadPath(lpFileName.data(), MAX_PATH);
      else
        GetExecutePath(lpFileName.data(), MAX_PATH);
      wcscat_s(lpFileName.data(), lpFileName.size(), L"\\ehnd_log.log");

      auto file = wofstream{lpFileName.data(), ios::app};

      auto const time = zone->to_local(chrono::system_clock::now());
      auto ms = chrono::duration_cast<chrono::milliseconds>(time.time_since_epoch()).count() % 1000;
      file << format(L"{:%X}.{:03} | ", time, ms) << message;
    }
  }

  logLine++;
}
