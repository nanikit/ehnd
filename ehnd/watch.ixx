module;
#include <Windows.h>

export module Watch;

import std.core;

export class Watch {
 public:
  Watch(std::function<MMRESULT(std::vector<std::wstring>& paths)> callback);
  ~Watch();
  void TurnOn();
  void TurnOff();

 private:
  bool bWatch = true;
  HANDLE hWatchThread;
  std::function<MMRESULT(std::vector<std::wstring>& paths)> callback_;
  std::vector<std::wstring> files_;

  DWORD ReadDirectoryChanges();
  MMRESULT Callback();

  static DWORD WINAPI WatcherThreadInvoker(LPVOID self) {
    return reinterpret_cast<Watch*>(self)->ReadDirectoryChanges();
  }

  static MMRESULT CALLBACK Poller(UINT m_nTimerID, UINT uiMsg, DWORD dwUser, DWORD dw1, DWORD d2) {
    return reinterpret_cast<Watch*>(dwUser)->Callback();
  }
};
