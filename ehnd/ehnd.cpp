#include "stdafx.h"

#include "globals.h"

FARPROC apfnEzt[100];
FARPROC apfnMsv[100];
int g_initTick;

bool EhndInit() {
  using namespace std;

  // 중복 초기화 방지
  static bool initOnce = false;
  if (initOnce)
    return false;
  else
    initOnce = true;

  locale::global(locale{".65001"});
  ios::sync_with_stdio(false);

  // init ehnd
  pFilter = new Filter();
  pWatch = new Watch();
  pConfig = new Config();

  SetLogText(L"EhndInit : 이지트랜스 초기화\n");

  char szInitTick[12];
  g_initTick = GetTickCount() + rand();
  _itoa_s(g_initTick, szInitTick, 10);

  GetTempPathA(MAX_PATH, g_DicPath);
  strcat_s(g_DicPath, "UserDict_");
  strcat_s(g_DicPath, szInitTick);
  strcat_s(g_DicPath, ".ehnd");

  // 설정 로드
  pConfig->LoadConfig();

  // 기존 로그 삭제
  if (pConfig->GetFileLogStartupClear()) {
    wstring file_name;
    if (pConfig->GetFileLogEztLoc()) {
      file_name = pConfig->GetEhndPath();
    } else {
      file_name.resize(MAX_PATH);
      GetExecutePath(file_name.data(), file_name.size());
      file_name.resize(wcsnlen(file_name.data(), file_name.size()));
    }
    file_name += L"\\ehnd_log.log";
    DeleteFile(file_name.c_str());
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

  Log(LogCategory::kNormal, L"HookUserDict : 사용자사전 알고리즘 최적화.\n");

  // 엔드 임시파일 삭제
  pFilter->ehnddic_cleanup();

  pFilter->load();
  return true;
}

// 이지트랜스 API
__declspec(naked) void J2K_Initialize(void) {
  __asm JMP apfnEzt[4 * 0];
}

bool __stdcall J2K_InitializeEx(LPCSTR name, LPCSTR key) {
  if (!EhndInit()) {
    return false;
  }

  __asm {
    PUSH DWORD PTR DS : [key]
    PUSH name
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

LPSTR TranslateMMNT(LPCSTR jpn) {
  using namespace std;

  static mutex mtx;
  auto lock = lock_guard<mutex>{mtx};

  LPSTR szKor = nullptr;
  // J2KEngine.dll's TranslateMMNT increases ESP by 1 and it matches with no calling convention.
  __asm {
    PUSH DWORD PTR DS : [jpn]
    PUSH 0
    CALL apfnEzt[4 * 18]
    MOV DWORD PTR DS : [szKor], EAX
  }

  return szKor;
}

template <typename T>
class EztransMemory {
 public:
  EztransMemory(T memory) : memory_(memory) {
    static_assert(std::is_pointer<T>::value, "EztransMemory should be a pointer.");
  }

  T data() {
    return memory_;
  }

  virtual ~EztransMemory() {
    msvcrt_free(memory_);
  }

 private:
  T memory_;
};

std::string WideToMultiByteWithEscape(std::wstring_view source, UINT codePage) {
  using namespace std;

  string dest;
  BOOL hasUnconvertible = false;
  int length = WideCharToMultiByteWithAral(codePage, 0, source.data(), source.size(), nullptr, 0,
                                           nullptr, &hasUnconvertible);

  if (hasUnconvertible) {
    dest.reserve(length);

    array<char, 4> buf{};
    for (auto& ch : source) {
      length = WideCharToMultiByteWithAral(codePage, 0, &ch, 1, buf.data(), buf.size(), nullptr,
                                           &hasUnconvertible);
      if (hasUnconvertible) {
        // Anemone uses this, so naively followed.
        dest += format("+x{:04X}", static_cast<int>(ch));
      } else {
        dest.append(buf.data(), length);
      }
    }
  } else {
    dest.resize(length);
    WideCharToMultiByteWithAral(codePage, 0, source.data(), -1, dest.data(), dest.size(), nullptr,
                                nullptr);
  }

  return dest;
}

std::wstring MultiByteToWide(std::string_view source, UINT codePage, bool useOriginal = false,
                             const std::optional<std::wstring>& buffer = std::nullopt) {
  using namespace std;

  auto mb_to_wc = useOriginal ? MultiByteToWideChar : MultiByteToWideCharWithAral;

  int i_len = mb_to_wc(codePage, 0, source.data(), source.size(), nullptr, 0);
  wstring dest{move(buffer.value_or(wstring{}))};
  dest.resize(i_len);
  mb_to_wc(codePage, 0, source.data(), source.size(), dest.data(), dest.size());

  return dest;
}

const char* FindHexEscape(std::string_view input) {
  using namespace std;

  auto search_end = end(input) - min(static_cast<int>(input.size()), 5);
  for (auto iter = begin(input); iter < search_end; iter++) {
    if (iter[0] == '+' && iter[1] == 'x') {
      if (all_of(iter + 2, iter + 6, [](char c) { return !!isxdigit(c); })) {
        return &*iter;
      }
    }
  }
  return nullptr;
}

std::wstring MultiByteToWideAndUnescape(std::string_view source, UINT codePage,
                                        const std::optional<std::wstring>& buffer = std::nullopt) {
  using namespace std;

  const auto source_end = source.data() + source.size();
  auto start = source.data();
  wstring buf;
  wstring dest{move(buffer.value_or(wstring{}))};
  dest.clear();

  while (true) {
    auto text_end = FindHexEscape({start, source_end});
    if (text_end == nullptr) {
      text_end = source_end;
    }

    buf = MultiByteToWide({start, text_end}, codePage, false, move(buf));
    dest += buf;

    if (text_end == source_end) {
      break;
    }

    unsigned short hex;
    if (from_chars(&text_end[2], &text_end[6], hex, 16).ec == errc{}) {
      dest += static_cast<wchar_t>(hex);
    }

    start = text_end + 6;
  }

  return dest;
}

// 亮介(같은 문자열은 세그폴트를 일으켜 번역을 중단시킨다.
// 그런 이유로 줄 수가 부족하면 이어서 번역
std::string TranslateWithRecovery(const std::string& jpn) {
  using namespace std;

  string kor;
  auto start = jpn.c_str();

  while (true) {
    EztransMemory sz_kor{TranslateMMNT(start)};

    string_view span = sz_kor.data();
    kor += span;

    auto line_feed_count = count(begin(span), end(span), L'\n');
    auto next_line = boost::find_nth(start, "\n", line_feed_count).end();
    if (!*next_line) {
      break;
    }

    start = next_line;
    kor += '\n';
  }

  return kor;
}

std::wstring TranslateAndMeasureTime(std::wstring&& text) {
  using namespace std;

  if (!pConfig->GetUserDicSwitch()) {
    Log(LogCategory::kNormal, L"UserDic : 사용자 사전이 꺼져 있습니다.\n");
  }

  string jpn = WideToMultiByteWithEscape(text, 932);

  auto tick_start = GetTickCount64();

  string kor = TranslateWithRecovery(jpn);

  auto tick_end = GetTickCount64();

  Log(LogCategory::kTime, L"J2K_TranslateMMNT : --- Elasped Time : {}ms ---\n",
      tick_end - tick_start);
  text = MultiByteToWideAndUnescape(kor, 949, move(text));

  return text;
}

std::wstring FilterAndTranslate(std::wstring&& text) {
  using namespace std;

  pFilter->pre(text);
  Log(LogCategory::kNormal, L"[PRE] {}\n\n", text);

  text = TranslateAndMeasureTime(move(text));

  Log(LogCategory::kNormal, L"[TRANS] {}\n\n", text);
  pFilter->post(text);
  Log(LogCategory::kNormal, L"[POST] {}\n\n", text);

  return text;
}

wchar_t* AllocateNullTerminatedSharedMemory(std::wstring& text) {
  using namespace std;

  auto pShareable = static_cast<LPWSTR>(CoTaskMemAlloc((text.size() + 1) * sizeof(text[0])));
  if (!pShareable) {
    Log(LogCategory::kError, L"J2K_TranslateMMNT : Memory Allocation Error.\n");
    return nullptr;
  }

  span<wchar_t> szOut{pShareable, text.size() + 1};
  text.copy(szOut.data(), text.size());
  szOut[text.size()] = L'\0';
  return szOut.data();
}

wchar_t* TranslateMMNTW(LPCWSTR szIn) {
  using namespace std;

  // 로그 크기 체크
  CheckLogSize();

  // 콘솔 라인 체크
  CheckConsoleLine();

  wstring_view original{szIn};
  wstring text{original};

  // 넘어온 문자열의 길이가 0이거나 명령어일때 번역 프로세스 스킵
  if (text.size()) {
    Log(LogCategory::kNormal, L"[REQUEST] {}\n\n", text);
    if (pFilter->cmd(text)) {
      Log(LogCategory::kNormal, L"[COMMAND] {}\n\n", text);
    } else {
      text = FilterAndTranslate(move(text));
    }
  }

  return AllocateNullTerminatedSharedMemory(text);
}

wchar_t* __stdcall J2K_TranslateMMNTW(int data0, LPCWSTR szIn) {
  using namespace std;

  try {
    return TranslateMMNTW(szIn);
  } catch (exception ex) {
    auto message = MultiByteToWide(ex.what(), CP_ACP);
    Log(LogCategory::kError, L"J2K_TranslateMMNT : {}\n", message);
  } catch (...) {
    Log(LogCategory::kError, L"J2K_TranslateMMNT : Unknown error\n");
  }
  return nullptr;
}

void* __stdcall J2K_TranslateMMNT(int data0, LPCSTR szIn) {
  using namespace std;

  LPSTR szOut;
  wstring wsText, wsOriginal;
  int i_len;
  LPWSTR lpJPN, lpKOR;

  lpJPN = 0;
  i_len = MultiByteToWideCharWithAral(932, MB_PRECOMPOSED, szIn, -1, NULL, NULL);
  lpJPN = (LPWSTR)msvcrt_malloc((i_len + 1) * 3);
  if (lpJPN == NULL) {
    Log(LogCategory::kError, L"J2K_TranslateMMNT : Memory Allocation Error.\n");
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
    Log(LogCategory::kError, L"J2K_TranslateMMNT : Memory Allocation Error.\n");
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
