#include "stdafx.h"

#include "config.h"
#include "ehnd.h"
#include "log.h"

Config::Config() {
  wcscpy_s(cfg_console_fontname, L"굴림");
}

Config::~Config() {
}

const wchar_t* const Config::kEngineDllSubPath = L"\\J2KEngine.dlx";

bool DoesDlxExist(const std::wstring& directory) {
  auto dll = directory + Config::kEngineDllSubPath;
  return std::filesystem::exists(dll);
}

std::wstring GetEztransPathFromRegistry() {
  using namespace std;
  using namespace winreg;

  RegKey key;
  auto openResult = key.TryOpen(HKEY_CURRENT_USER, L"Software\\ChangShin\\ezTrans");
  if (!openResult) {
    return {};
  }

  auto getResult = key.TryGetStringValue(L"FilePath");
  if (!getResult) {
    return {};
  }

  auto& ehndPath = getResult.GetValue();
  return DoesDlxExist(ehndPath) ? ehndPath : std::wstring{};
}

std::wstring Config::GetEztransPath() {
  using namespace std;

  auto path = GetEhndPath();
  if (DoesDlxExist(path)) {
    return path;
  }

  path = GetEztransPathFromRegistry();
  if (path.size()) {
    return path;
  }

  auto defaultPath = LR"(C:\Program Files (x86)\ChangShinSoft\ezTrans XP)";
  path = defaultPath;
  if (DoesDlxExist(path)) {
    return path;
  }

  return {};
}

std::wstring Config::GetEhndPath() {
  using namespace std;

  int flag = GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT;
  auto address = reinterpret_cast<LPCTSTR>(&ShowLogWin);
  HMODULE handle;
  if (!GetModuleHandleEx(flag, address, &handle)) {
    int ret = GetLastError();
    Log(LogCategory::kError, L"GetModuleHandleEx failed, error {}\n", ret);
    return {};
  }

  wstring path;
  path.resize(MAX_PATH);
  int length = GetModuleFileName(handle, path.data(), path.size());
  if (!length) {
    int ret = GetLastError();
    Log(LogCategory::kError, L"GetModuleFileName failed, error {}\n", ret);
    return {};
  }

  path.resize(path.rfind(L'\\'));
  return path;
}

std::wstring Config::GetExecutablePath() {
  using namespace std;

  wstring path;
  path.resize(MAX_PATH);
  int length = GetModuleFileName(GetModuleHandle(nullptr), path.data(), path.size());
  if (!length) {
    int ret = GetLastError();
    Log(LogCategory::kError, L"GetModuleFileName(nullptr) failed, error {}\n", ret);
    return {};
  }

  path.resize(path.rfind(L'\\'));
  return path;
}

bool Config::LoadConfig() {
  using namespace std;

  array<wchar_t, 255> buf{};
  auto ini_path = GetEhndPath() + L"\\Ehnd\\ehnd_conf.ini";

  auto get_config = [&](LPCWSTR key) -> optional<wstring_view> {
    int length =
      GetPrivateProfileStringW(L"CONFIG", key, nullptr, buf.data(), buf.size(), ini_path.c_str());
    return length ? make_optional(wstring_view(buf.data(), length)) : nullopt;
  };

  auto get_switch = [&](LPCWSTR key) -> bool {
    return !boost::iequals(get_config(key).value_or(L""), L"OFF");
  };

  SetPreSwitch(get_switch(L"PREFILTER_SWITCH"));
  SetPostSwitch(get_switch(L"POSTFILTER_SWITCH"));
  SetJKDICSwitch(get_switch(L"JKDIC_SWITCH"));

  SetUserDicSwitch(get_switch(L"USERDIC_SWITCH"));
  SetEhndWatchSwitch(get_switch(L"EHNDWATCH_SWITCH"));
  SetCommandSwitch(get_switch(L"COMMAND_SWITCH"));

  SetLogDetail(get_switch(L"LOG_DETAIL"));
  SetLogTime(get_switch(L"LOG_TIME"));
  SetLogSkipLayer(get_switch(L"LOG_SKIPLAYER"));
  SetLogUserDic(get_switch(L"LOG_USERDIC"));

  auto get_int = [&](LPCWSTR key) -> optional<int> {
    return get_config(key).transform(&wstring_view::data).transform(_wtoi);
  };

  SetFileLogSwitch(get_switch(L"FILELOG_SWITCH"));
  SetFileLogSize(get_int(L"FILELOG_SIZE").value_or(300));
  SetFileLogEztLoc(get_switch(L"FILELOG_EZTRANS_LOC"));
  SetFileLogStartupClear(get_switch(L"FILELOG_STARTUP_CLEAR"));

  SetConsoleSwitch(get_switch(L"CONSOLE_SWITCH"));
  SetConsoleMaxLine(get_int(L"CONSOLE_MAXLINE").value_or(300));
  SetConsoleFontName(
    get_config(L"CONSOLE_FONTNAME").transform(&wstring_view::data).value_or(L"굴림"));
  SetConsoleFontSize(get_int(L"CONSOLE_FONTSIZE").value_or(12));

  if (!firstInit)
    Log(LogCategory::kNormal, L"LoadConfig : Success.\n");
  else
    firstInit = false;
  return true;
}

bool Config::SaveConfig() {
  wchar_t INIPath[MAX_PATH], buf[255];
  GetLoadPath(INIPath, MAX_PATH);
  wcscat_s(INIPath, L"\\Ehnd\\ehnd_conf.ini");

  // wcscpy_s(buf, (GetPreSwitch() ? L"ON" : L"OFF"));
  // WriteINI(L"PREFILTER_SWITCH", L"CONFIG", buf, (wchar_t*)INIPath);
  // wcscpy_s(buf, (GetPostSwitch() ? L"ON" : L"OFF"));
  // WriteINI(L"POSTFILTER_SWITCH", L"CONFIG", buf, (wchar_t*)INIPath);
  // wcscpy_s(buf, (GetJKDICSwitch() ? L"ON" : L"OFF"));
  // WriteINI(L"JKDIC_SWITCH", L"CONFIG", buf, (wchar_t*)INIPath);
  wcscpy_s(buf, (GetCommandSwitch() ? L"ON" : L"OFF"));
  WriteINI(L"COMMAND_SWITCH", L"CONFIG", buf, (wchar_t*)INIPath);

  wcscpy_s(buf, (GetLogDetail() ? L"ON" : L"OFF"));
  WriteINI(L"LOG_DETAIL", L"CONFIG", buf, (wchar_t*)INIPath);
  wcscpy_s(buf, (GetLogTime() ? L"ON" : L"OFF"));
  WriteINI(L"LOG_TIME", L"CONFIG", buf, (wchar_t*)INIPath);
  wcscpy_s(buf, (GetLogSkipLayer() ? L"ON" : L"OFF"));
  WriteINI(L"LOG_SKIPLAYER", L"CONFIG", buf, (wchar_t*)INIPath);
  wcscpy_s(buf, (GetLogUserDic() ? L"ON" : L"OFF"));
  WriteINI(L"LOG_USERDIC", L"CONFIG", buf, (wchar_t*)INIPath);

  wcscpy_s(buf, (GetFileLogSwitch() ? L"ON" : L"OFF"));
  WriteINI(L"FILELOG_SWITCH", L"CONFIG", buf, (wchar_t*)INIPath);
  wsprintf(buf, L"%d", GetFileLogSize());
  WriteINI(L"FILELOG_SIZE", L"CONFIG", buf, (wchar_t*)INIPath);
  wcscpy_s(buf, (GetFileLogEztLoc() ? L"ON" : L"OFF"));
  WriteINI(L"FILELOG_EZTRANS_LOC", L"CONFIG", buf, (wchar_t*)INIPath);
  wcscpy_s(buf, (GetFileLogStartupClear() ? L"ON" : L"OFF"));
  WriteINI(L"FILELOG_STARTUP_CLEAR", L"CONFIG", buf, (wchar_t*)INIPath);

  wcscpy_s(buf, (GetConsoleSwitch() ? L"ON" : L"OFF"));
  WriteINI(L"CONSOLE_SWITCH", L"CONFIG", buf, (wchar_t*)INIPath);
  wsprintf(buf, L"%d", GetConsoleMaxLine());
  WriteINI(L"CONSOLE_MAXLINE", L"CONFIG", buf, (wchar_t*)INIPath);
  wsprintf(buf, L"%s", GetConsoleFontName());
  WriteINI(L"CONSOLE_FONTNAME", L"CONFIG", buf, (wchar_t*)INIPath);
  wsprintf(buf, L"%d", GetConsoleFontSize());
  WriteINI(L"CONSOLE_FONTSIZE", L"CONFIG", buf, (wchar_t*)INIPath);

  Log(LogCategory::kNormal, L"SaveConfig : Success.\n");
  return true;
}

bool Config::WriteINI(const wchar_t* key, const wchar_t* section, wchar_t* buf, wchar_t* file) {
  int n = WritePrivateProfileString(section, key, buf, file);
  if (n == 0) return false;
  return true;
}
