module;
#include <Windows.h>

export module Log;

import std.core;
import std.filesystem;

import Config;

export {
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
}

int logLine = 0;

export template <typename... Args>
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
      auto file = wofstream{pConfig->GetFileLogDirectory() + L"\\ehnd_log.log", ios::app};

      auto const time = zone->to_local(chrono::system_clock::now());
      auto ms = chrono::duration_cast<chrono::milliseconds>(time.time_since_epoch()).count() % 1000;
      file << format(L"{:%X}.{:03} | ", time, ms) << message;
    }
  }

  logLine++;
}
