#pragma once
#include <Windows.h>

#include <string>
#include "log.h"

class LoggedConfig : public Config {
  void SetConsoleSwitch(bool b) override {
    Config::SetConsoleSwitch(b);
    ShowLogWin(b);
  }

  const std::wstring GetEhndPath() override {
    auto path = Config::GetEhndPath();
    if (path.empty()) {
      int ret = GetLastError();
      Log(LogCategory::kError, L"GetEhndPath failed, error {}\n", ret);
    }
    return path;
  }

  const std::wstring GetExecutablePath() override {
    auto path = Config::GetExecutablePath();
    if (path.empty()) {
      int ret = GetLastError();
      Log(LogCategory::kError, L"GetModuleHandleEx failed, error {}\n", ret);
    }
    return path;
  }

  bool LoadConfig() override {
    bool success = Config::LoadConfig();
    if (!first_init_) {
      Log(LogCategory::kNormal, L"LoadConfig : Success.\n");
    } else {
      first_init_ = false;
    }
    return success;
  }

  bool SaveConfig() override {
    bool success = Config::SaveConfig();
    Log(LogCategory::kNormal, L"SaveConfig : Success.\n");
    return success;
  }

 private:
  bool first_init_ = true;
};