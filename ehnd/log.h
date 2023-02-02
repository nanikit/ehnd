#pragma once

#include <winnt.h>
#include <array>
#include <boost/locale.hpp>
#include <chrono>
#include <format>
#include <fstream>

#include "config.h"

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

extern config* pConfig;
extern int logLine;

template <typename... Args>
auto Log(log_category category, const std::_Fmt_wstring<Args...> fmt, Args&&... args) {
  using namespace std;
  static auto zone = chrono::current_zone();
  static auto loc = boost::locale::generator().generate("UTF-8");

  if (!pConfig->GetLogTime() && category == log_category::time) return;
  if (!pConfig->GetLogDetail() && category == log_category::detail) return;
  if (!pConfig->GetLogSkipLayer() && category == log_category::skip_layer) return;
  if (!pConfig->GetLogUserDic() && category == log_category::user_dict) return;

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
      file.imbue(loc);

      auto const time = zone->to_local(chrono::system_clock::now());
      auto ms = chrono::duration_cast<chrono::milliseconds>(time.time_since_epoch()).count() % 1000;
      file << format(L"{:%X}.{:03} | ", time, ms) << message;
    }
  }

  logLine++;
}
