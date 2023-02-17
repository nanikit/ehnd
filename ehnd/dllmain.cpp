// dllmain.cpp : DLL 응용 프로그램의 진입점을 정의합니다.
#include "stdafx.h"

#include "globals.h"

HINSTANCE g_hInst;
Filter* pFilter;
Watch* pWatch;
Config* pConfig;
HMODULE hEzt, hMsv;
BOOL g_bAnemone = false;

BOOL APIENTRY DllMain(HINSTANCE hInstance, DWORD ul_reason_for_call, LPVOID lpReserved) {
  switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
      g_hInst = hInstance;
      break;
    case DLL_THREAD_ATTACH:
      break;
    case DLL_THREAD_DETACH:
      break;
    case DLL_PROCESS_DETACH:
      FreeLibrary(hEzt);
      FreeLibrary(hMsv);
      break;
  }
  return TRUE;
}
