#include "stdafx.h"

#include "globals.h"

using namespace std;

FARPROC apfnEzt[100];
FARPROC apfnMsv[100];
bool initOnce = false;

bool EhndInit(void) {
  // 중복 초기화 방지
  if (initOnce)
    return false;
  else
    initOnce = true;

  // 설정 로드
  pConfig->LoadConfig();

  // 기존 로그 삭제
  if (pConfig->GetFileLogStartupClear()) {
    wchar_t lpFileName[MAX_PATH];
    if (pConfig->GetFileLogEztLoc())
      GetLoadPath(lpFileName, MAX_PATH);
    else
      GetExecutePath(lpFileName, MAX_PATH);
    wcscat_s(lpFileName, L"\\ehnd_log.log");
    DeleteFile(lpFileName);
  }

  CreateLogWin(g_hInst);
  ShowLogWin(pConfig->GetConsoleSwitch());
  LogStartMsg();

  GetRealWC2MB();
  GetRealMB2WC();

  if (!hook()) return false;
  if (!hook_userdict()) return false;
  if (!hook_userdict2()) return false;
  if (!hook_getwordinfo()) return false;

  Log(log_category::normal, L"HookUserDict : 사용자사전 알고리즘 최적화.\n");

  // 엔드 임시파일 삭제
  pFilter->ehnddic_cleanup();

  pFilter->load();
  return true;
}

// 이지트랜스 API
__declspec(naked) void J2K_Initialize(void) {
  __asm JMP apfnEzt[4 * 0];
}
void __stdcall J2K_InitializeEx(int data0, LPSTR key) {
  SetLogText(L"J2K_InitializeEx : 이지트랜스 초기화\n");

  EhndInit();
  __asm {
		PUSH DWORD PTR DS : [key]
		PUSH data0
		CALL apfnEzt[4 * 1]
  }
}

void __stdcall J2K_FreeMem(void* buffer) {
  CoTaskMemFree(buffer);
}

__declspec(naked) void J2K_GetPriorDict(void) {
  __asm JMP apfnEzt[4 * 3];
}
__declspec(naked) void J2K_GetProperty(void) {
  __asm JMP apfnEzt[4 * 4];
}
void __stdcall J2K_ReloadUserDict(void) {
  pFilter->load_dic();
  __asm CALL apfnEzt[4 * 5];
  return;
}
__declspec(naked) void J2K_SetDelJPN(void) {
  __asm JMP apfnEzt[4 * 6];
}
__declspec(naked) void J2K_SetField(void) {
  __asm JMP apfnEzt[4 * 7];
}
__declspec(naked) void J2K_SetHnj2han(void) {
  __asm JMP apfnEzt[4 * 8];
}
__declspec(naked) void J2K_SetJWin(void) {
  __asm JMP apfnEzt[4 * 9];
}
__declspec(naked) void J2K_SetPriorDict(void) {
  __asm JMP apfnEzt[4 * 10];
}
__declspec(naked) void J2K_SetProperty(void) {
  __asm JMP apfnEzt[4 * 11];
}
__declspec(naked) void J2K_StopTranslation(void) {
  __asm JMP apfnEzt[4 * 12];
}
__declspec(naked) void J2K_Terminate(void) {
  __asm JMP apfnEzt[4 * 13];
}
__declspec(naked) void J2K_TranslateChat(void) {
  __asm JMP apfnEzt[4 * 14];
}
__declspec(naked) void J2K_TranslateFM(void) {
  __asm JMP apfnEzt[4 * 15];
}
__declspec(naked) void J2K_TranslateMM(void) {
  __asm JMP apfnEzt[4 * 16];
}
__declspec(naked) void J2K_TranslateMMEx(void) {
  __asm JMP apfnEzt[4 * 17];
}
__declspec(naked) void* msvcrt_free(void* _Memory) {
  __asm JMP apfnMsv[4 * 0];
}
__declspec(naked) void* msvcrt_malloc(size_t _Size) {
  __asm JMP apfnMsv[4 * 1];
}
__declspec(naked) void* msvcrt_fopen(char* path, char* mode) {
  __asm JMP apfnMsv[4 * 2];
}

void FilterAndTranslate(std::wstring& text) {
  pFilter->pre(text);

  Log(log_category::normal, L"[PRE] {}\n\n", text);

  int i_len = WideCharToMultiByteWithAral(932, 0, text.c_str(), -1, nullptr, 0, nullptr, nullptr);
  string jpn;
  jpn.resize(i_len);

  WideCharToMultiByteWithAral(932, 0, text.c_str(), -1, jpn.data(), jpn.size(), NULL, NULL);

  if (!pConfig->GetUserDicSwitch()) {
    Log(log_category::normal, L"UserDic : 사용자 사전이 꺼져 있습니다.\n");
  }

  auto tickStart = GetTickCount64();

  // TranslateMMNT increases ESP by 1 and it matches with no calling convention.
  LPCSTR szJpn = jpn.c_str();
  LPCSTR szKor = "";
  __asm {
			PUSH DWORD PTR DS : [szJpn]
			PUSH 0
			CALL apfnEzt[4 * 18]
			MOV DWORD PTR DS : [szKor], EAX
  }

  auto tickEnd = GetTickCount64();

  Log(log_category::time, L"J2K_TranslateMMNT : --- Elasped Time : {}ms ---\n",
      tickEnd - tickStart);

  i_len = MultiByteToWideCharWithAral(949, 0, szKor, -1, nullptr, 0);
  text.resize(i_len);
  MultiByteToWideCharWithAral(949, 0, szKor, -1, text.data(), text.size());
  text.resize(max(0, text.size() - 1));

  Log(log_category::normal, L"[TRANS] {}\n\n", text);

  pFilter->post(text);

  Log(log_category::normal, L"[POST] {}\n\n", text);
}

wchar_t* AllocateNullTerminatedSharedMemory(std::wstring& text) {
  auto pShareable = static_cast<LPWSTR>(CoTaskMemAlloc((text.size() + 1) * sizeof(text[0])));
  if (!pShareable) {
    Log(log_category::error, L"J2K_TranslateMMNT : Memory Allocation Error.\n");
    return nullptr;
  }

  span<wchar_t> szOut{pShareable, text.size() + 1};
  text.copy(szOut.data(), text.size());
  szOut[text.size()] = L'\0';
  return szOut.data();
}

wchar_t* TranslateMMNTW(LPCWSTR szIn) {
  // 로그 크기 체크
  CheckLogSize();

  // 콘솔 라인 체크
  CheckConsoleLine();

  wstring_view original{szIn};
  wstring text{original};

  // 넘어온 문자열의 길이가 0이거나 명령어일때 번역 프로세스 스킵
  if (text.size()) {
    Log(log_category::normal, L"[REQUEST] {}\n\n", text);
    if (pFilter->cmd(text)) {
      Log(log_category::normal, L"[COMMAND] {}\n\n", text);
    } else {
      FilterAndTranslate(text);
    }
  }

  return AllocateNullTerminatedSharedMemory(text);
}

void* __stdcall J2K_TranslateMMNTW(int data0, LPCWSTR szIn) {
  try {
    return TranslateMMNTW(szIn);
  } catch (exception ex) {
    mbstate_t state{};
    auto message = string_view{ex.what()};
    wstring dest;
    dest.resize(message.size());
    auto pt = message.data();
    size_t count = 0;
    mbsrtowcs_s(&count, dest.data(), dest.size(), &pt, dest.size(), &state);
    Log(log_category::error, L"J2K_TranslateMMNT : {}\n", dest);
  } catch (...) {
    Log(log_category::error, L"J2K_TranslateMMNT : Unknown error\n");
  }
  return nullptr;
}

void* __stdcall J2K_TranslateMMNT(int data0, LPCSTR szIn) {
  LPSTR szOut;
  wstring wsText, wsOriginal;
  int i_len;
  LPWSTR lpJPN, lpKOR;

  lpJPN = 0;
  i_len = MultiByteToWideCharWithAral(932, MB_PRECOMPOSED, szIn, -1, NULL, NULL);
  lpJPN = (LPWSTR)msvcrt_malloc((i_len + 1) * 3);
  if (lpJPN == NULL) {
    Log(log_category::error, L"J2K_TranslateMMNT : Memory Allocation Error.\n");
    return 0;
  }
  MultiByteToWideCharWithAral(932, 0, szIn, -1, lpJPN, i_len);
  wsText = lpJPN;
  msvcrt_free(lpJPN);

  lpKOR = (LPWSTR)J2K_TranslateMMNTW(data0, wsText.c_str());

  // cp949 내보내기
  i_len = WideCharToMultiByteWithAral(949, 0, lpKOR, -1, NULL, NULL, NULL, NULL);
  szOut = static_cast<LPSTR>(CoTaskMemAlloc((i_len + 1) * 3));
  if (szOut == NULL) {
    Log(log_category::error, L"J2K_TranslateMMNT : Memory Allocation Error.\n");
    return 0;
  }
  WideCharToMultiByteWithAral(949, 0, lpKOR, -1, szOut, i_len, NULL, NULL);
  msvcrt_free(lpKOR);

  return szOut;
}
__declspec(naked) void J2K_GetJ2KMainDir(void) {
  __asm JMP apfnEzt[4 * 19];
}

bool GetLoadPath(LPWSTR Path, int Size) {
  GetModuleFileName(g_hInst, Path, Size);
  if (Path[0] == 0) return false;
  int i = wcslen(Path);
  while (i--) {
    if (Path[i] == L'\\') {
      Path[i] = 0;
      break;
    }
  }
  return true;
}

bool GetExecutePath(LPWSTR Path, int Size) {
  GetModuleFileName(GetModuleHandle(NULL), Path, Size);
  if (Path[0] == 0) return false;
  int i = wcslen(Path);
  while (i--) {
    if (Path[i] == L'\\') {
      Path[i] = 0;
      break;
    }
  }
  return true;
}

wstring replace_all(const wstring& str, const wstring& pattern, const wstring& replace) {
  wstring result = str;
  wstring::size_type pos = 0, offset = 0;

  while ((pos = result.find(pattern, offset)) != wstring::npos) {
    result.replace(result.begin() + pos, result.begin() + pos + pattern.size(), replace);
    offset = pos + replace.size();
  }
  return result;
}

wstring deformatted_string(const wstring& str) {
  wchar_t pattern[][2] = {L"%", L"\\"};
  wchar_t replace[][3] = {L"%%", L"\\\\"};
  wstring result = str;
  wstring::size_type pos = 0, offset = 0;

  for (int i = 0; i < 1; i++) {
    while ((pos = result.find(pattern[i], offset)) != wstring::npos) {
      result.replace(result.begin() + pos, result.begin() + pos + 1, replace[i]);
      offset = pos + 2;
    }
  }
  return result;
}
