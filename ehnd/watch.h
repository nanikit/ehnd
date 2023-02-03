#pragma once

#include <Windows.h>

class Watch {
 public:
  Watch();
  ~Watch();
  void TurnOn();
  void TurnOff();

 private:
  static Watch* m_pThis;
  HANDLE hWatchThread;

  bool bWatch = true;

  DWORD _NotifyThread(LPVOID lpParam);
  MMRESULT _NotifyProc(UINT m_nTimerID, UINT uiMsg, DWORD dwUser, DWORD dw1, DWORD d2);

  // Trampoline
  static DWORD WINAPI NotifyThread(LPVOID lpParam) {
    return m_pThis->_NotifyThread(lpParam);
  }

  static MMRESULT CALLBACK NotifyProc(UINT m_nTimerID, UINT uiMsg, DWORD dwUser, DWORD dw1,
                                      DWORD d2) {
    return m_pThis->_NotifyProc(m_nTimerID, uiMsg, dwUser, dw1, d2);
  }
};
