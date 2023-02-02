// dllmain.cpp : DLL 응용 프로그램의 진입점을 정의합니다.
#include "stdafx.h"

#include "globals.h"

HINSTANCE g_hInst;
filter* pFilter;
watch* pWatch;
config* pConfig;
HMODULE hEzt, hMsv;
int g_initTick;
char g_DicPath[MAX_PATH];
BOOL g_bAnemone = false;

BOOL APIENTRY DllMain(HINSTANCE hInstance, DWORD ul_reason_for_call, LPVOID lpReserved) {
  switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
      g_hInst = hInstance;
      std::ios::sync_with_stdio(false);

      // init ehnd
      pFilter = new filter();
      pWatch = new watch();
      pConfig = new config();

      char szInitTick[12];
      g_initTick = GetTickCount() + rand();
      _itoa_s(g_initTick, szInitTick, 10);

      GetTempPathA(MAX_PATH, g_DicPath);
      strcat_s(g_DicPath, "UserDict_");
      strcat_s(g_DicPath, szInitTick);
      strcat_s(g_DicPath, ".ehnd");

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