#include "stdafx.h"

#include "globals.h"

using namespace std;
using namespace chrono;
using namespace boost;

int operator<(USERDICSTRUCT& left, USERDICSTRUCT& right) {
  char buffer1[62], buffer2[62];
  int result =
    WideCharToMultiByteWithAral(932, 0, left._jpn, -1, buffer1, sizeof(buffer1), nullptr, nullptr);
  if (!result) {
    return 0;
  }

  result =
    WideCharToMultiByteWithAral(932, 0, right._jpn, -1, buffer2, sizeof(buffer2), nullptr, nullptr);
  if (!result) {
    return 0;
  }

  return strcmp(buffer1, buffer2) > 0 ||
         ((strcmp(buffer1, buffer2) == 0) && left._type < right._type) ||
         ((strcmp(buffer1, buffer2) == 0) && (left._type == right._type) &&
          (left.g_line < right.g_line));
}

filter::filter() {
  hLoadEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
}

filter::~filter() {
  CloseHandle(hLoadEvent);
}

bool filter::load() {
  WaitForSingleObject(hLoadEvent, INFINITE);
  if (!pre_load() || !post_load() || !userdic_load() || skiplayer_load()) {
    SetEvent(hLoadEvent);
    return false;
  }
  SetEvent(hLoadEvent);
  return true;
}

bool filter::load_dic() {
  WaitForSingleObject(hLoadEvent, INFINITE);
  if (!userdic_load()) {
    SetEvent(hLoadEvent);
    return false;
  }
  SetEvent(hLoadEvent);
  return true;
}

bool filter::pre_load() {
  WCHAR lpEztPath[MAX_PATH];
  WIN32_FIND_DATA FindFileData;
  FILTERSTRUCT fs;
  wstring Path;

  DWORD dwStart, dwEnd;
  dwStart = GetTickCount();

  GetLoadPath(lpEztPath, MAX_PATH);
  Path = lpEztPath;
  Path += L"\\Ehnd\\PreFilter*.txt";

  int pre_line = 1;

  vector<FILTERSTRUCT> Filter;

  HANDLE hFind = FindFirstFile(Path.c_str(), &FindFileData);

  do {
    if (hFind == INVALID_HANDLE_VALUE)
      break;
    else if (FindFileData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)
      continue;

    Path = lpEztPath;
    Path += L"\\Ehnd\\";

    filter_load(Filter, Path.c_str(), FindFileData.cFileName, rule_type::preprocess, pre_line);

  } while (FindNextFile(hFind, &FindFileData));

  // 정렬
  sort(Filter.begin(), Filter.end());
  // WriteLog(log_category::normal, L"PreFilterRead : 필터 정렬을 완료했습니다.\n");

  // 필터 대체
  PreFilter = Filter;
  WriteLog(log_category::normal, L"PreFilterRead : 총 %d개의 전처리 필터를 읽었습니다.\n",
           PreFilter.size());

  // 소요시간 계산
  dwEnd = GetTickCount();
  WriteLog(log_category::time, L"PreFilterRead : --- Elasped Time : %dms ---\n", dwEnd - dwStart);
  return true;
}

bool filter::post_load() {
  WCHAR lpEztPath[MAX_PATH];
  WIN32_FIND_DATA FindFileData;
  FILTERSTRUCT fs;
  wstring Path;

  DWORD dwStart, dwEnd;
  dwStart = GetTickCount();

  GetLoadPath(lpEztPath, MAX_PATH);
  Path = lpEztPath;
  Path += L"\\Ehnd\\PostFilter*.txt";

  int post_line = 1;

  vector<FILTERSTRUCT> Filter;

  HANDLE hFind = FindFirstFile(Path.c_str(), &FindFileData);

  do {
    if (hFind == INVALID_HANDLE_VALUE)
      break;
    else if (FindFileData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)
      continue;

    Path = lpEztPath;
    Path += L"\\Ehnd\\";

    filter_load(Filter, Path.c_str(), FindFileData.cFileName, rule_type::postprocess, post_line);

  } while (FindNextFile(hFind, &FindFileData));

  // 정렬
  sort(Filter.begin(), Filter.end());
  // WriteLog(log_category::normal, L"PostFilterRead : 필터 정렬을 완료했습니다.\n");

  // 필터 대체
  PostFilter = Filter;
  WriteLog(log_category::normal, L"PostFilterRead : 총 %d개의 후처리 필터를 읽었습니다.\n",
           PostFilter.size());

  // 소요시간 계산
  dwEnd = GetTickCount();
  WriteLog(log_category::time, L"PostFilterRead : --- Elasped Time : %dms ---\n", dwEnd - dwStart);
  return true;
}

bool filter::skiplayer_load() {
  WCHAR lpEztPath[MAX_PATH];
  WIN32_FIND_DATA FindFileData;
  FILTERSTRUCT fs;
  wstring Path;

  DWORD dwStart, dwEnd;
  dwStart = GetTickCount();

  GetLoadPath(lpEztPath, MAX_PATH);
  Path = lpEztPath;
  Path += L"\\Ehnd\\SkipLayer*.txt";

  int skiplayer_line = 1;

  vector<SKIPLAYERSTRUCT> _SkipLayer;

  HANDLE hFind = FindFirstFile(Path.c_str(), &FindFileData);

  do {
    if (hFind == INVALID_HANDLE_VALUE)
      break;
    else if (FindFileData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)
      continue;

    Path = lpEztPath;
    Path += L"\\Ehnd\\";

    skiplayer_load2(_SkipLayer, Path.c_str(), FindFileData.cFileName, skiplayer_line);

  } while (FindNextFile(hFind, &FindFileData));

  // 정렬
  sort(_SkipLayer.begin(), _SkipLayer.end());
  // WriteLog(log_category::normal, L"SkipLayerRead : 스킵레이어 정렬을 완료했습니다.\n");

  // 스킵레이어 대체
  SkipLayer = _SkipLayer;
  WriteLog(log_category::normal, L"SkipLayerRead : 총 %d개의 스킵레이어를 읽었습니다.\n",
           SkipLayer.size());

  // 소요시간 계산
  dwEnd = GetTickCount();
  WriteLog(log_category::time, L"SkipLayerRead : --- Elasped Time : %dms ---\n", dwEnd - dwStart);
  return true;
}
bool filter::userdic_load() {
  WCHAR lpEztPath[MAX_PATH];
  WIN32_FIND_DATA FindFileData;
  wstring Path;

  DWORD dwStart, dwEnd;
  dwStart = GetTickCount();

  GetLoadPath(lpEztPath, MAX_PATH);
  Path = lpEztPath;
  Path += L"\\Ehnd\\UserDict*.txt";

  int userdic_line = 1;
  UserDic.clear();

  // load userdict.jk
  if (pConfig->GetJKDICSwitch()) jkdic_load(userdic_line);

  // load anedic.txt
  anedic_load(userdic_line);

  HANDLE hFind = FindFirstFile(Path.c_str(), &FindFileData);

  do {
    if (hFind == INVALID_HANDLE_VALUE)
      break;
    else if (FindFileData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)
      continue;

    Path = lpEztPath;
    Path += L"\\Ehnd\\";

    userdic_load2(Path.c_str(), FindFileData.cFileName, userdic_line);

  } while (FindNextFile(hFind, &FindFileData));

  // 정렬
  sort(UserDic.begin(), UserDic.end());
  // WriteLog(log_category::normal, L"UserDicRead : 사용자 사전 정렬을 완료했습니다.\n");

  WriteLog(log_category::normal, L"UserDicRead : 총 %d개의 사용자 사전 파일을 읽었습니다.\n",
           UserDic.size());

  // 소요시간 계산
  dwEnd = GetTickCount();
  WriteLog(log_category::time, L"UserDicRead : --- Elasped Time : %dms ---\n", dwEnd - dwStart);

  // 엔드 임시파일 생성
  ehnddic_create();
  return true;
}

bool filter::jkdic_load(int& g_line) {
  WCHAR lpEztPath[MAX_PATH];
  CHAR Jpn[32], Kor[32], Part[6], Attr[38], Hidden;
  CHAR Buffer[1024];
  FILE* fp;
  wstring Path;
  int line = 1;
  wchar_t lpBuffer[128];

  DWORD dwStart, dwEnd;
  dwStart = GetTickCount();

  GetLoadPath(lpEztPath, MAX_PATH);
  Path = lpEztPath;
  Path += L"\\Dat\\UserDict.jk";

  if (_wfopen_s(&fp, Path.c_str(), L"rb") != 0) {
    WriteLog(log_category::normal,
             L"JkDicLoad : DAT 사용자 사전 파일 \"UserDict.jk\"이 없습니다.\n");

    // userdict.jk 파일이 없으면 빈 파일 만듦
    if (_wfopen_s(&fp, Path.c_str(), L"wb") == 0) fclose(fp);
    return false;
  }

  //
  // ezTrans XP UserDict.jk Struct
  // [0] hidden (1 byte)
  // [1-31] jpn (31 bytes)
  // [32-62] kor (31 bytes)
  // [63-67] part of speech (5 bytes)
  // [68-109] attributes (42 bytes)
  //
  USERDICSTRUCT us;
  int vaild_line = 0;
  while (1) {
    Jpn[31] = 0, Kor[31] = 0, Part[5] = 0, Attr[37] = 0;

    if (!fread(Buffer, sizeof(char), 1, fp))
      break;
    else
      (Buffer[0] == 0x00) ? Hidden = false : Hidden = true;

    if (!fread(Buffer, sizeof(char), 31, fp))
      break;
    else
      memcpy(Jpn, Buffer, 31);

    if (!fread(Buffer, sizeof(char), 31, fp))
      break;
    else
      memcpy(Kor, Buffer, 31);

    if (!fread(Buffer, sizeof(char), 5, fp))
      break;
    else
      memcpy(Part, Buffer, 5);

    if (!fread(Buffer, sizeof(char), 37, fp))
      break;
    else
      memcpy(Attr, Buffer, 37);
    memcpy(Attr + 37, L"", 1);

    if (!fread(Buffer, sizeof(char), 5, fp)) break;

    int len;
    len = MultiByteToWideCharWithAral(932, MB_PRECOMPOSED, Jpn, -1, NULL, NULL);
    MultiByteToWideCharWithAral(932, 0, Jpn, -1, lpBuffer, len);
    wcsncpy_s(us._jpn, lpBuffer, len);

    len = MultiByteToWideCharWithAral(949, MB_PRECOMPOSED, Kor, -1, NULL, NULL);
    MultiByteToWideCharWithAral(949, 0, Kor, -1, lpBuffer, len);
    wcsncpy_s(us._kor, lpBuffer, len);

    len = MultiByteToWideCharWithAral(949, MB_PRECOMPOSED, Attr, -1, NULL, NULL);
    MultiByteToWideCharWithAral(949, 0, Attr, -1, lpBuffer, len);
    wcsncpy_s(us._attr, lpBuffer, len);

    wcscpy_s(us._db, L"UserDict.jk");

    if (!strcmp(Part, "A9D0"))
      us._type = user_word::common;
    else
      us._type = user_word::noun;

    us.g_line = g_line;
    us.line = line;

    if (!Hidden) UserDic.push_back(us);
    line++, g_line++;
  }
  fclose(fp);

  WriteLog(log_category::normal,
           L"JkDicRead : %d개의 DAT 사용자 사전 \"UserDict.jk\"를 읽었습니다.\n", line - 1);

  // 소요시간 계산
  dwEnd = GetTickCount();
  WriteLog(log_category::time, L"JkDicRead : --- Elasped Time : %dms ---\n", dwEnd - dwStart);
  return true;
}

bool filter::ehnddic_cleanup() {
  WCHAR lpTmpPath[MAX_PATH];
  WIN32_FIND_DATA FindFileData;
  wstring Path;

  DWORD dwStart, dwEnd;
  dwStart = GetTickCount();

  GetTempPath(MAX_PATH, lpTmpPath);
  Path = lpTmpPath;
  Path += L"\\UserDict*.ehnd";

  HANDLE hFind = FindFirstFile(Path.c_str(), &FindFileData);

  do {
    WriteLog(log_category::normal, L"EhndDicCleanUp : %s\n", D(FindFileData.cFileName));
    if (hFind == INVALID_HANDLE_VALUE)
      break;
    else if (FindFileData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)
      continue;

    int i = wcslen(FindFileData.cFileName) - 1;
    while (i--) {
      if (FindFileData.cFileName[i] == L'.' && !wcscmp(FindFileData.cFileName + i, L".ehnd")) {
        wstring DelFile = lpTmpPath;
        DelFile += FindFileData.cFileName;
        DeleteFile(DelFile.c_str());
        break;
      }
    }

  } while (FindNextFile(hFind, &FindFileData));

  // 소요시간 계산
  dwEnd = GetTickCount();
  WriteLog(log_category::time, L"EhndDicCleanUp : --- Elasped Time : %dms ---\n", dwEnd - dwStart);

  return true;
}

bool filter::ehnddic_create() {
  WCHAR lpTmpPath[MAX_PATH], lpText[12];
  CHAR Jpn[32], Kor[32], Part[6], Attr[38];
  wstring Path;
  FILE* fp;
  DWORD dwStart, dwEnd;
  _itow_s(g_initTick, lpText, 10);
  dwStart = GetTickCount();

  GetTempPath(MAX_PATH, lpTmpPath);
  Path = lpTmpPath;
  Path += L"\\UserDict_";
  Path += lpText;
  Path += L".ehnd";

  if (_wfopen_s(&fp, Path.c_str(), L"wb") != 0)
    WriteLog(
      log_category::normal,
      L"EhndDicCreate : 사용자사전 바이너리 \"UserDict_%s.ehnd\" 파일을 생성하는데 실패했습니다.\n",
      lpText);

  for (UINT i = 0; i < UserDic.size(); i++) {
    // 공백 제거
    if (UserDic[i]._jpn == 0) continue;

    memset(Jpn, 0, sizeof(Jpn));
    memset(Kor, 0, sizeof(Kor));
    memset(Attr, 0, sizeof(Attr));

    int len;

    // 유니코드 -> 932/949
    len = WideCharToMultiByteWithAral(932, 0, UserDic[i]._jpn, -1, NULL, NULL, NULL, NULL);
    WideCharToMultiByteWithAral(932, 0, UserDic[i]._jpn, -1, Jpn, len, NULL, NULL);
    len = WideCharToMultiByteWithAral(949, 0, UserDic[i]._kor, -1, NULL, NULL, NULL, NULL);
    WideCharToMultiByteWithAral(949, 0, UserDic[i]._kor, -1, Kor, len, NULL, NULL);
    len = WideCharToMultiByteWithAral(949, 0, UserDic[i]._attr, -1, NULL, NULL, NULL, NULL);
    WideCharToMultiByteWithAral(949, 0, UserDic[i]._attr, -1, Attr, len, NULL, NULL);

    // 단어 타입
    if (UserDic[i]._type == user_word::common)
      strcpy_s(Part, "A9D0");
    else
      strcpy_s(Part, "I110");

    fwrite(L"", sizeof(char), 1, fp);
    fwrite(Jpn, sizeof(char), 31, fp);
    fwrite(Kor, sizeof(char), 31, fp);
    fwrite(Part, sizeof(char), 5, fp);
    fwrite(Attr, sizeof(char), 37, fp);

    // UserDic_Log 구현을 위해 NULL+attr 끝 네자리를 활용
    // 기존 attr 배열은 42자까지 허용되었으나 끝자리를 사용하는 관계로 36자까지만 허용 (1자는 여유)
    fwrite(L"", sizeof(char), 1, fp);
    fwrite(&i, sizeof(char), 4, fp);
  }
  WriteLog(log_category::normal,
           L"EhndDicCreate : 사용자사전 바이너리 \"UserDict_%s.ehnd\" 생성.\n", lpText);
  fclose(fp);

  // 소요시간 계산
  dwEnd = GetTickCount();
  WriteLog(log_category::time, L"EhndDicCreate : --- Elasped Time : %dms ---\n", dwEnd - dwStart);
  return true;
}

bool filter::skiplayer_load2(vector<SKIPLAYERSTRUCT>& SkipLayer, LPCWSTR lpPath, LPCWSTR lpFileName,
                             int& g_line) {
  WCHAR Buffer[1024], Context[1024];
  FILE* fp;

  wstring Path;
  Path = lpPath;
  Path += lpFileName;
  int vaild_line = 0;

  if (_wfopen_s(&fp, Path.c_str(), L"rt,ccs=UTF-8") != 0) {
    WriteLog(log_category::normal, L"SkipLayerRead : 스킵레이어 %s 로드 실패.\n", D(lpFileName));
    return false;
  }
  // WriteLog(log_category::normal, L"SkipLayerRead : 스킵레이어 %s 로드.\n", lpFileName);

  for (int line = 0; fgetws(Buffer, 1000, fp) != NULL; line++, g_line++) {
    int tab = 0;
    if (Buffer[0] == L'/' && Buffer[1] == L'/') continue;

    SKIPLAYERSTRUCT ss;
    ss.line = line;

    for (UINT i = 0, prev = 0; i <= wcslen(Buffer) + 1; i++) {
      if (Buffer[i] == L'\t' || Buffer[i] == L'\n' ||
          (Buffer[i] == L'/' && Buffer[i - 1] == L'/') || i == wcslen(Buffer)) {
        switch (tab) {
          case 0:
            wcsncpy_s(Context, Buffer + prev, i - prev);
            tab++;
            prev = i + 1;
            ss.wtype = Context;
            if (!wcsncmp(Context, L"PRE", 3))
              ss.type = rule_type::preprocess;
            else if (!wcsncmp(Context, L"POST", 4))
              ss.type = rule_type::postprocess;
            else
              i = wcslen(Buffer) + 1;
            break;
          case 1:
            wcsncpy_s(Context, Buffer + prev, i - prev);
            tab++;
            prev = i + 1;
            ss.layer = _wtoi(Context);
            ss.wlayer = Context;
            break;
          case 2:
            wcsncpy_s(Context, Buffer + prev, i - prev);
            tab++;
            prev = i + 1;
            ss.cond = Context;
            break;
        }
      }
    }
    if (tab < 2) continue;

    try {
      wregex ex(ss.cond);
    } catch (regex_error ex) {
      WCHAR lpWhat[255];
      int len = MultiByteToWideCharWithAral(949, MB_PRECOMPOSED, ex.what(), -1, NULL, NULL);
      MultiByteToWideCharWithAral(949, MB_PRECOMPOSED, ex.what(), -1, lpWhat, len);

      WriteLog(log_category::error, L"SkipLayerRead : 정규식 오류! : [%s:%d] %s | %s | %s\n",
               L"SkipLayer.txt", line, D(ss.wtype), D(ss.wlayer), D(ss.cond));

      continue;
    }
    vaild_line++;
    SkipLayer.push_back(ss);
  }
  fclose(fp);
  WriteLog(log_category::normal, L"SkipLayerRead : %d개의 스킵레이어 %s를 읽었습니다.\n",
           vaild_line, D(lpFileName));
  return true;
}

bool filter::filter_load(vector<FILTERSTRUCT>& Filter, LPCWSTR lpPath, LPCWSTR lpFileName,
                         rule_type rule_type, int& g_line) {
  FILE* fp;
  WCHAR Buffer[1024], Context[1024];

  wstring Path;
  Path = lpPath;
  Path += lpFileName;
  int vaild_line = 0;

  if (_wfopen_s(&fp, Path.c_str(), L"rt,ccs=UTF-8") != 0) {
    if (rule_type == rule_type::preprocess)
      WriteLog(log_category::normal, L"PreFilterRead : 전처리 필터 '%s' 로드 실패!\n",
               D(lpFileName));
    if (rule_type == rule_type::postprocess)
      WriteLog(log_category::normal, L"PostFilterRead : 후처리 필터 '%s' 로드 실패!\n",
               D(lpFileName));
    return false;
  }

  /*
  if (rule_type == rule_type::preprocess && IsUnicode) WriteLog(log_category::normal,
  L"PreFilterRead : 전처리 유니코드 전용 필터 \"%s\" 로드.\n", lpFileName); if (rule_type ==
  rule_type::preprocess && !IsUnicode) WriteLog(log_category::normal, L"PreFilterRead : 전처리 필터
  \"%s\" 로드.\n", lpFileName); else if (rule_type == rule_type::postprocess && IsUnicode)
  WriteLog(log_category::normal, L"PostFilterRead : 후처리 유니코드 전용 필터 \"%s\" 로드.\n",
  lpFileName); else if (rule_type == rule_type::postprocess && !IsUnicode)
  WriteLog(log_category::normal, L"PostFilterRead : 후처리 필터 \"%s\" 로드.\n", lpFileName);
  */
  for (int line = 0; fgetws(Buffer, 1000, fp) != NULL; line++, g_line++) {
    if (Buffer[0] == L'/' && Buffer[1] == L'/') continue;  // 주석

    FILTERSTRUCT fs;
    fs.g_line = g_line;
    fs.line = line;
    fs.db = lpFileName;
    fs.src = L"";
    fs.dest = L"";

    int tab = 0;
    for (UINT i = 0, prev = 0; i <= wcslen(Buffer); i++) {
      if (Buffer[i] == L'\t' || Buffer[i] == L'\n' ||
          (Buffer[i] == L'/' && Buffer[i - 1] == L'/') || i == wcslen(Buffer)) {
        switch (tab) {
          case 0:
            wcsncpy_s(Context, Buffer + prev, i - prev);
            prev = i + 1;
            tab++;
            fs.src = Context;
            break;
          case 1:
            wcsncpy_s(Context, Buffer + prev, i - prev);
            prev = i + 1;
            tab++;
            fs.dest = Context;
            break;
          case 2:
            wcsncpy_s(Context, Buffer + prev, i - prev);
            prev = i + 1;
            tab++;
            fs.layer = _wtoi(Context);
            break;
          case 3:
            wcsncpy_s(Context, Buffer + prev, i - prev);
            prev = i + 1;
            tab++;
            fs.regex = _wtoi(Context);
            break;
        }
        if (Buffer[i] == L'/' && Buffer[i - 1] == L'/') break;
      }
    }

    if (tab < 3) continue;
    if (fs.regex == 1) {
      try {
        wregex ex(fs.src);
      } catch (regex_error ex) {
        WCHAR lpWhat[255];
        int len = MultiByteToWideCharWithAral(949, MB_PRECOMPOSED, ex.what(), -1, NULL, NULL);
        MultiByteToWideCharWithAral(949, MB_PRECOMPOSED, ex.what(), -1, lpWhat, len);

        if (rule_type == rule_type::preprocess)
          WriteLog(log_category::error,
                   L"PreFilterRead : 정규식 오류! : [%s:%d] %s | %s | %d | %d\n", D(lpFileName),
                   line, D(fs.src), D(fs.dest), fs.layer, fs.regex);
        else if (rule_type == rule_type::postprocess)
          WriteLog(log_category::error,
                   L"PostFilterRead : 정규식 오류! : [%s:%d] %s | %s | %d | %d\n", D(lpFileName),
                   line, D(fs.src), D(fs.dest), fs.layer, fs.regex);
        continue;
      }
    }
    vaild_line++;

    // initialize cound and elasped time variable
    fs._ecount = 0;
    fs._etime = 0;

    Filter.push_back(fs);
  }
  fclose(fp);

  if (rule_type == rule_type::preprocess)
    WriteLog(log_category::normal, L"PreFilterRead : %d개의 전처리 필터 \"%s\"를 읽었습니다.\n",
             vaild_line, D(lpFileName));
  else if (rule_type == rule_type::postprocess)
    WriteLog(log_category::normal, L"PostFilterRead : %d개의 후처리 필터 \"%s\"를 읽었습니다.\n",
             vaild_line, D(lpFileName));
  return true;
}

bool filter::userdic_load2(LPCWSTR lpPath, LPCWSTR lpFileName, int& g_line) {
  FILE* fp;
  WCHAR Buffer[1024], Context[1024];
  wstring Path, Jpn, Kor, Attr;
  int count, len;
  Path = lpPath;
  Path += lpFileName;

  if (_wfopen_s(&fp, Path.c_str(), L"rt,ccs=UTF-8") != 0) {
    WriteLog(log_category::normal, L"UserDicRead : 사용자 사전 '%s' 로드 실패!\n", D(lpFileName));
    return false;
  }

  // WriteLog(log_category::normal, L"UserDicRead : 사용자 사전 \"%s\" 로드.\n", lpFileName);

  count = 0;
  for (int line = 1; fgetws(Buffer, 1000, fp) != NULL; line++) {
    if (!wcsncmp(Buffer, L"//", 2)) continue;
    if (Buffer[wcslen(Buffer) - 1] == 0x0A) Buffer[wcslen(Buffer) - 1] = 0;
    if (Buffer[wcslen(Buffer) - 1] == 0x0D) Buffer[wcslen(Buffer) - 1] = 0;

    USERDICSTRUCT us;
    memset(us._jpn, 0, sizeof(us._jpn));
    memset(us._kor, 0, sizeof(us._kor));
    memset(us._attr, 0, sizeof(us._attr));
    memset(us._db, 0, sizeof(us._db));

    us._type = user_word::noun;

    int t = 0, n = 0;
    wchar_t* pStr = Buffer;
    while (*pStr != 0) {
      if (*pStr == L'\t' || (!wcsncmp(pStr, L"//", 2)) || *(pStr + 1) == 0) {
        if (*(pStr + 1) == 0)
          wcsncpy_s(Context, pStr - n, n + 1);
        else
          wcsncpy_s(Context, pStr - n, n);

        if (t < 4) {
          if (t == 0)
            Jpn = Context;
          else if (t == 1)
            Kor = Context;
          else if (t == 2)
            if ((Context[0] == L'0' || Context[0] == L'2') && Context[1] == 0)
              us._type = user_word::common;
            else
              us._type = user_word::noun;
          else if (t == 3)
            Attr = Context;

          if (!wcsncmp(pStr, L"//", 2)) break;
        }

        pStr++, n = 0, t++;
      } else
        pStr++, n++;
    }

    if (t > 1) {
      if ((len = WideCharToMultiByteWithAral(932, 0, Jpn.c_str(), -1, NULL, NULL, NULL, NULL)) >
          31) {
        WriteLog(log_category::normal,
                 L"UserDicRead : 오류 : 원문 단어의 길이는 15자(30Byte)를 초과할 수 없습니다.\n");
        WriteLog(log_category::normal,
                 L"UserDicRead : 오류 : 다음 단어가 무시됩니다. (현재: %dByte)\n", len);
        WriteLog(log_category::normal, L"UserDicRead : 오류 : [%s:%d] : <<%s>> | %s | %x | %s\n",
                 D(lpFileName), line, D(Jpn), D(Kor), us._type, D(Attr));
        t = 0;
        break;
      }
      wcscpy_s(us._jpn, Jpn.c_str());
      if ((len = WideCharToMultiByteWithAral(949, 0, Kor.c_str(), -1, NULL, NULL, NULL, NULL)) >
          31) {
        WriteLog(log_category::normal,
                 L"UserDicRead : 오류 : 역문 단어의 길이는 15자(30Byte)를 초과할 수 없습니다.\n");
        WriteLog(log_category::normal,
                 L"UserDicRead : 오류 : 다음 단어가 무시됩니다. (현재: %dByte)\n", len);
        WriteLog(log_category::normal, L"UserDicRead : 오류 : [%s:%d] : %s | <<%s>> | %x | %s\n",
                 D(lpFileName), line, D(Jpn), D(Kor), us._type, D(Attr));
        t = 0;
        break;
      }
      wcscpy_s(us._kor, Kor.c_str());
      if ((len = WideCharToMultiByteWithAral(949, 0, Attr.c_str(), -1, NULL, NULL, NULL, NULL)) >
          37) {
        WriteLog(log_category::normal,
                 L"UserDicRead : 오류 : 단어 속성은 36Byte를 초과할 수 없습니다.\n");
        WriteLog(log_category::normal,
                 L"UserDicRead : 오류 : 다음 단어가 무시됩니다. (현재: %dByte)\n", len);
        WriteLog(log_category::normal, L"UserDicRead : 오류 : [%s:%d] : %s | %s | %x | <<%s>>\n",
                 D(lpFileName), line, D(Jpn), D(Kor), us._type, D(Attr));
        t = 0;
        break;
      }
      wcscpy_s(us._attr, Attr.c_str());

      wcscpy_s(us._db, lpFileName);
      us.line = line;
      us.g_line = g_line;
      g_line++, count++;
      UserDic.push_back(us);
    }
  }
  WriteLog(log_category::normal, L"UserDicRead : %d개의 사용자 사전 \"%s\"를 읽었습니다.\n", count,
           D(lpFileName));
  fclose(fp);
  return true;
}

// anedic.txt 호환을 위한 함수
// 필터가 변경이 되면 anedic.txt 파일을 찾아 읽는다
bool filter::anedic_load(int& g_line) {
  wstring Jpn, Kor, Attr;
  WCHAR lpPathName[MAX_PATH], lpFileName[MAX_PATH];

  if (!g_bAnemone) {
    DWORD pid;
    HWND hwnd = FindWindow(L"AneParentClass", NULL);
    if (hwnd) {
      // WriteLog(log_category::normal, L"[AneDicLoad] AneParentClass Found.\n");

      GetWindowThreadProcessId(hwnd, &pid);
      if (GetCurrentProcessId() != pid) {
        // WriteLog(log_category::normal, L"[AneDicLoad] GetCurrentProcessId() != pid.\n");
        return false;
      }
    } else {
      hwnd = FindWindow(L"AnemoneParentWndClass", NULL);
      if (hwnd) {
        // WriteLog(log_category::normal, L"[AneDicLoad] AnemoneParentWndClass Found.\n");
        GetWindowThreadProcessId(hwnd, &pid);
        if (GetCurrentProcessId() != pid) {
          // WriteLog(log_category::normal, L"[AneDicLoad] GetCurrentProcessId() != pid.\n");
          return false;
        }
      } else {
        // WriteLog(log_category::normal, L"[AneDicLoad] Anemone Not Found\n");
        return false;
      }
    }
    g_bAnemone = true;
  }

  GetExecutePath(lpPathName, MAX_PATH);
  wcscat_s(lpPathName, L"\\");
  wcscpy_s(lpFileName, L"anedic.txt");

  return userdic_load2(lpPathName, lpFileName, g_line);
}

bool filter::pre(wstring& wsText) {
  if (!pConfig->GetPreSwitch()) {
    WriteLog(log_category::normal, L"PreFilter : 전처리가 꺼져 있습니다.\n");
    return false;
  }
  return filter_proc(PreFilter, rule_type::preprocess, wsText);
}

bool filter::post(wstring& wsText) {
  if (!pConfig->GetPostSwitch()) {
    WriteLog(log_category::normal, L"PostFilter : 후처리가 꺼져 있습니다.\n");
    return false;
  }
  return filter_proc(PostFilter, rule_type::postprocess, wsText);
}

bool filter::filter_proc(vector<FILTERSTRUCT>& Filter, rule_type rule_type, wstring& wsText) {
  DWORD dwStart, dwEnd;
  system_clock::time_point start, end;
  typedef duration<double, milli> doubleMilli;
  wstring Str = wsText;
  int layer_prev = -1;
  bool layer_pass = false;
  bool pass_once = false;
  wregex ex;

  wstring pass_log;
  dwStart = GetTickCount();

  for (UINT i = 0; i < Filter.size(); i++) {
    // 저장된 차수와 현재 필터의 차수가 다를 때 조건식을 돌려 PASS 여부 확인
    if (Filter[i].layer != layer_prev) {
      layer_prev = Filter[i].layer;
      layer_pass = false;

      for (UINT j = 0; j < SkipLayer.size(); j++) {
        if (SkipLayer[j].type == rule_type && SkipLayer[j].layer == Filter[i].layer) {
          try {
            ex.assign(SkipLayer[j].cond);

            if (!regex_search(wsText, ex)) {
              layer_pass = true;
              pass_once = true;
              pass_log += (SkipLayer[j].wlayer + L", ");
              break;
            }
          } catch (regex_error ex) {
            WCHAR lpWhat[255];
            int len = MultiByteToWideCharWithAral(949, MB_PRECOMPOSED, ex.what(), -1, NULL, NULL);
            MultiByteToWideCharWithAral(949, MB_PRECOMPOSED, ex.what(), -1, lpWhat, len);

            WriteLog(log_category::error, L"SkipLayerRead : 정규식 오류! : [%s:%d] %s | %d | %s\n",
                     L"SkipLayer.txt", SkipLayer[i].line, D(SkipLayer[i].wtype), SkipLayer[i].layer,
                     D(SkipLayer[i].cond));
            continue;
          }
        }
      }
    }
    if (Filter[i].layer == layer_prev && layer_pass) continue;

    if (!Filter[i].regex) {
      start = system_clock::now();
      Filter[i]._ecount++;

      Str = wsText;
      wsText = replace_all(wsText, Filter[i].src, Filter[i].dest);
      if (Str.compare(wsText)) {
        if (rule_type == rule_type::preprocess)
          WriteLog(log_category::detail, L"PreFilter : [%s:%d] | %s | %s | %d | %d\n",
                   D(Filter[i].db), Filter[i].line, D(Filter[i].src), D(Filter[i].dest),
                   Filter[i].layer, Filter[i].regex);
        else if (rule_type == rule_type::postprocess)
          WriteLog(log_category::detail, L"PostFIlter : [%s:%d] | %s | %s | %d | %d\n",
                   D(Filter[i].db), Filter[i].line, D(Filter[i].src), D(Filter[i].dest),
                   Filter[i].layer, Filter[i].regex);
      }

      end = system_clock::now();
      Filter[i]._etime += duration_cast<doubleMilli>(end - start).count();
    } else {
      Str = wsText;

      try {
        start = system_clock::now();
        ex.assign(Filter[i].src);

        if (regex_search(wsText, ex)) {
          Filter[i]._ecount++;
          wsText = regex_replace(wsText, ex, Filter[i].dest, regex_constants::match_default);
        }
        end = system_clock::now();
        Filter[i]._etime += duration_cast<doubleMilli>(end - start).count();
      } catch (regex_error ex) {
        WCHAR lpWhat[255];
        int len = MultiByteToWideCharWithAral(949, MB_PRECOMPOSED, ex.what(), -1, NULL, NULL);
        MultiByteToWideCharWithAral(949, MB_PRECOMPOSED, ex.what(), -1, lpWhat, len);

        if (rule_type == rule_type::preprocess)
          WriteLog(log_category::error, L"PreFilter : 정규식 오류! : [%s:%d] %s | %s | %d | %d\n",
                   D(Filter[i].db), Filter[i].line, D(Filter[i].src), D(Filter[i].dest),
                   Filter[i].layer, Filter[i].regex);
        else if (rule_type == rule_type::postprocess)
          WriteLog(log_category::error, L"PostFilter : 정규식 오류! : [%s:%d] %s | %s | %d | %d\n",
                   D(Filter[i].db), Filter[i].line, D(Filter[i].src), D(Filter[i].dest),
                   Filter[i].layer, Filter[i].regex);
        continue;
      }

      if (Str != wsText) {
        if (rule_type == rule_type::preprocess)
          WriteLog(log_category::detail, L"PreFilter : [%s:%d] %s | %s | %d | %d\n",
                   D(Filter[i].db), Filter[i].line, D(Filter[i].src), D(Filter[i].dest),
                   Filter[i].layer, Filter[i].regex);
        else if (rule_type == rule_type::postprocess)
          WriteLog(log_category::detail, L"PostFilter : [%s:%d] %s | %s | %d | %d\n",
                   D(Filter[i].db), Filter[i].line, D(Filter[i].src), D(Filter[i].dest),
                   Filter[i].layer, Filter[i].regex);
        Str = wsText;
      }
    }
  }

  pass_log = pass_log.substr(0, pass_log.rfind(L", "));

  if (pass_once) {
    if (rule_type == rule_type::preprocess)
      WriteLog(log_category::skip_layer, L"PreSkipLayer : %s\n", pass_log.c_str());
    else if (rule_type == rule_type::postprocess)
      WriteLog(log_category::skip_layer, L"PostSkipLayer : %s\n", pass_log.c_str());
  }

  dwEnd = GetTickCount();
  if (rule_type == rule_type::preprocess)
    WriteLog(log_category::time, L"PreFilter : --- Elasped Time : %dms ---\n", dwEnd - dwStart);
  else if (rule_type == rule_type::postprocess)
    WriteLog(log_category::time, L"PostFIlter : --- Elasped Time : %dms ---\n", dwEnd - dwStart);
  return true;
}

bool filter::cmd(wstring& wsText) {
  BOOL bCommand = false;
  BOOL bSaveINI = false;
  if (wsText[0] != L'/') return false;
  if (!wsText.compare(L"/ver") || !wsText.compare(L"/version")) {
    wsText += L" : Ehnd ";
    wsText += EHND_VER;
    wsText += L", ";
    wsText += _T(__DATE__);
    wsText += L", ";
    wsText += _T(__TIME__);
    wsText += L"\r\n";

    SetLogText(wsText.c_str(), RGB(168, 25, 25), RGB(255, 255, 255));
    return true;
  } else if (!wsText.compare(L"/log")) {
    if (IsShownLogWin()) {
      pConfig->SetConsoleSwitch(false);
      wsText = L"/log : Log Window Off.";
    } else {
      pConfig->SetConsoleSwitch(true);
      wsText = L"/log : Log Window On.";
    }
    bCommand = true;
  } else if (!wsText.compare(L"/command")) {
    if (pConfig->GetCommandSwitch()) {
      pConfig->SetCommandSwitch(false);
      wsText = L"/command : Command Off.";
    } else {
      pConfig->SetCommandSwitch(true);
      wsText = L"/command : Command On.";
    }
    bCommand = true;
    bSaveINI = true;
  } else if (!wsText.compare(L"/reload")) {
    pFilter->load();
    bCommand = true;
  } else if (pConfig->GetCommandSwitch()) {
    if (!wsText.compare(L"/log_detail")) {
      if (pConfig->GetLogDetail()) {
        pConfig->SetLogDetail(false);
        wsText = L"/log_detail : Detail Log Off.";
      } else {
        pConfig->SetLogDetail(true);
        wsText = L"/log_detail : Detail Log On.";
      }
      bCommand = true;
      bSaveINI = true;
    } else if (!wsText.compare(L"/log_time")) {
      if (pConfig->GetLogTime()) {
        pConfig->SetLogTime(false);
        wsText = L"/log_time : Time Log Off.";
      } else {
        pConfig->SetLogTime(true);
        wsText = L"/log_time : Time Log On.";
      }
      bCommand = true;
      bSaveINI = true;
    } else if (!wsText.compare(L"/log_skiplayer")) {
      if (pConfig->GetLogSkipLayer()) {
        pConfig->SetLogSkipLayer(false);
        wsText = L"/log_skiplayer : SkipLayer Log Off.";
      } else {
        pConfig->SetLogSkipLayer(true);
        wsText = L"/log_skiplayer : SkipLayer Log On.";
      }
      bCommand = true;
      bSaveINI = true;
    } else if (!wsText.compare(L"/log_userdic")) {
      if (pConfig->GetLogUserDic()) {
        pConfig->SetLogUserDic(false);
        wsText = L"/log_userdic : UserDic Log Off.";
      } else {
        pConfig->SetLogUserDic(true);
        wsText = L"/log_userdic : UserDic Log On.";
      }
      bCommand = true;
      bSaveINI = true;
    } else if (!wsText.compare(L"/filelog")) {
      if (pConfig->GetFileLogSwitch()) {
        pConfig->SetFileLogSwitch(false);
        wsText = L"/filelog : Write FileLog Off.";
      } else {
        pConfig->SetFileLogSwitch(true);
        wsText = L"/filelog : Write FileLog On.";
      }
      bCommand = true;
      bSaveINI = true;
    } else if (!wsText.compare(L"/preon") || !wsText.compare(L"/pre") && !pConfig->GetPreSwitch()) {
      pConfig->SetPreSwitch(true);
      wsText += L" : PreFilter On.";
      bCommand = true;
    }

    else if (!wsText.compare(L"/preoff") || !wsText.compare(L"/pre") && pConfig->GetPreSwitch()) {
      pConfig->SetPreSwitch(false);
      wsText += L" : PreFilter Off.";
      bCommand = true;
    }

    else if (!wsText.compare(L"/poston") ||
             !wsText.compare(L"/post") && !pConfig->GetPostSwitch()) {
      pConfig->SetPostSwitch(true);
      wsText += L" : PostFilter On.";
      bCommand = true;
    }

    else if (!wsText.compare(L"/postoff") ||
             !wsText.compare(L"/post") && pConfig->GetPostSwitch()) {
      pConfig->SetPostSwitch(false);
      wsText += L" : PostFilter Off.";
      bCommand = true;
    }

    else if (!wsText.compare(L"/dicon") ||
             !wsText.compare(L"/dic") && !pConfig->GetUserDicSwitch()) {
      pConfig->SetUserDicSwitch(true);
      wsText += L" : UserDic On.";
      bCommand = true;
    }

    else if (!wsText.compare(L"/dicoff") ||
             !wsText.compare(L"/dic") && pConfig->GetUserDicSwitch()) {
      pConfig->SetUserDicSwitch(false);
      wsText += L" : UserDic Off.";
      bCommand = true;
    }

    else if (!wsText.compare(L"/eout") && pConfig->GetUserDicSwitch()) {
      FILE* fp;
      wchar_t lpFileName[MAX_PATH];

      if (pConfig->GetFileLogEztLoc())
        GetLoadPath(lpFileName, MAX_PATH);
      else
        GetExecutePath(lpFileName, MAX_PATH);
      wcscat_s(lpFileName, L"\\ehnd_eout.log");

      if (!_wfopen_s(&fp, lpFileName, L"wt,ccs=UTF-8")) {
        for (size_t i = 0; i < PreFilter.size(); i++) {
          auto& Filter = PreFilter;
          fwprintf_s(fp, L"%s\t%d\t%d\t%f\t%s\t%s\t%d\t%d\n", D(Filter[i].db), Filter[i].line,
                     Filter[i]._ecount, Filter[i]._etime, D(Filter[i].src), D(Filter[i].dest),
                     Filter[i].layer, Filter[i].regex);
        }

        for (size_t i = 0; i < PostFilter.size(); i++) {
          auto& Filter = PostFilter;
          fwprintf_s(fp, L"%s\t%d\t%d\t%f\t%s\t%s\t%d\t%d\n", D(Filter[i].db), Filter[i].line,
                     Filter[i]._ecount, Filter[i]._etime, D(Filter[i].src), D(Filter[i].dest),
                     Filter[i].layer, Filter[i].regex);
        }
        fclose(fp);

        wsText += L" : Elasped Info Output.";
        bCommand = true;
      }
    }

    else if (!wsText.compare(L"/eclear") && pConfig->GetUserDicSwitch()) {
      for (size_t i = 0; i < PreFilter.size(); i++) {
        PreFilter[i]._ecount = 0;
        PreFilter[i]._etime = 0;
      }

      for (size_t i = 0; i < PostFilter.size(); i++) {
        PostFilter[i]._ecount = 0;
        PostFilter[i]._etime = 0;
      }

      wsText += L" : Elasped Info Clear.";
      bCommand = true;
    }
  }

  if (bSaveINI) pConfig->SaveConfig();
  if (bCommand) return true;
  return false;
}