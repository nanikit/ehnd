#include "stdafx.h"

#include "globals.h"

Watch* Watch::m_pThis = NULL;
std::vector<std::wstring> fileList;

Watch::Watch() {
  m_pThis = this;

  SECURITY_ATTRIBUTES ThreadAttributes;
  ThreadAttributes.bInheritHandle = false;
  ThreadAttributes.lpSecurityDescriptor = NULL;
  ThreadAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);

  hWatchThread = CreateThread(&ThreadAttributes, 0, NotifyThread, NULL, 0, NULL);
  if (hWatchThread == NULL) {
    Log(LogCategory::kError, L"WatchThread : WatchThread Create Error");
  }
}

Watch::~Watch() {
  TerminateThread(hWatchThread, 0);
}

void Watch::TurnOn() {
  bWatch = true;
}

void Watch::TurnOff() {
  bWatch = false;
}

DWORD Watch::_NotifyThread(LPVOID lpParam) {
  HWND hwnd = (HWND)lpParam;
  UINT m_nTimerID;
  WCHAR lpEztPath[MAX_PATH];
  std::wstring Path;
  GetLoadPath(lpEztPath, MAX_PATH);
  Path = lpEztPath;
  Path += L"\\Ehnd";

  HANDLE hDir = CreateFile(Path.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, 0,
                           OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, 0);
  CONST DWORD cbBuffer = 1024;
  BYTE pBuffer[1024];
  BOOL bWatchSubtree = FALSE;
  DWORD dwNotifyFilter = FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME |
                         FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_SIZE |
                         FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION;
  DWORD bytesReturned;

  m_nTimerID = timeSetEvent(500, 0, (LPTIMECALLBACK)NotifyProc, 0, TIME_PERIODIC);

  wchar_t temp[MAX_PATH] = {0};
  for (;;) {
    FILE_NOTIFY_INFORMATION* pfni;
    BOOL fOk = ReadDirectoryChangesW(hDir, pBuffer, cbBuffer, bWatchSubtree, dwNotifyFilter,
                                     &bytesReturned, 0, 0);
    if (!fOk) {
      DWORD dwLastError = GetLastError();
      break;
    }
    pfni = (FILE_NOTIFY_INFORMATION*)pBuffer;

    if (m_pThis->bWatch) {
      std::vector<std::wstring> fileList_Temp;
      do {
        memcpy(temp, pfni->FileName, pfni->FileNameLength);
        temp[pfni->FileNameLength / 2] = 0;
        std::wstring filename(temp);
        transform(filename.begin(), filename.end(), filename.begin(), towlower);

        if (fileList_Temp.begin() == fileList_Temp.end()) {
          fileList_Temp.push_back(filename.c_str());
        }

        std::vector<std::wstring>::iterator it = fileList_Temp.begin();
        for (; it != fileList_Temp.end(); it++) {
          if (it + 1 == fileList_Temp.end()) {
            fileList_Temp.push_back(filename.c_str());
            break;
          } else if ((*it).compare(filename.c_str()) == 0)
            break;
        }

        pfni = (FILE_NOTIFY_INFORMATION*)((PBYTE)pfni + pfni->NextEntryOffset);
      } while (pfni->NextEntryOffset > 0);
      fileList = fileList_Temp;
    }
  }
  return 0;
}

MMRESULT Watch::_NotifyProc(UINT m_nTimerID, UINT uiMsg, DWORD dwUser, DWORD dw1, DWORD d2) {
  bool c_prefilter = false;
  bool c_postfilter = false;
  bool c_userdic = false;
  bool c_config = false;
  bool c_skiplayer = false;

  if (fileList.begin() == fileList.end()) return 0;

  std::vector<std::wstring> fileList_Temp = fileList;
  fileList.clear();

  std::vector<std::wstring>::iterator it = fileList_Temp.begin();
  for (; it != fileList_Temp.end(); it++) {
    if ((*it).rfind(L".txt") != -1 && pConfig->GetEhndWatchSwitch()) {
      if ((*it).find(L"prefilter") != -1)
        c_prefilter = true;
      else if ((*it).find(L"postfilter") != -1)
        c_postfilter = true;
      else if ((*it).find(L"userdic") != -1)
        c_userdic = true;
      else if ((*it).find(L"skiplayer") != -1)
        c_skiplayer = true;
    } else if (!(*it).compare(L"ehnd_conf.ini"))
      c_config = true;
  }

  if (c_prefilter == true) {
    Log(LogCategory::kNormal, L"PreFilter : 전처리 필터 파일 변경사항 감지.\n");
    pFilter->pre_load();
  }

  if (c_postfilter == true) {
    Log(LogCategory::kNormal, L"PostFilter : 후처리 필터 파일 변경사항 감지.\n");
    pFilter->post_load();
  }

  if (c_skiplayer == true) {
    Log(LogCategory::kNormal, L"SkipLayer : 스킵 레이어 파일 변경사항 감지.\n");
    pFilter->skiplayer_load();
  }

  if (c_userdic == true) {
    Log(LogCategory::kNormal, L"UserDic : 사용자 사전 파일 변경사항 감지.\n");
    J2K_ReloadUserDict();
  }

  if (c_config == true) {
    Log(LogCategory::kNormal, L"Config : 설정파일 변경사항 감지.\n");
    pConfig->LoadConfig();
  }

  return 0;
}
