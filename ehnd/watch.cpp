module;
#include <Windows.h>

module Watch;

import std.core;
import Config;
import Log;

Watch::Watch(std::function<MMRESULT(std::vector<std::wstring>& paths)> callback) {
  callback_ = callback;

  SECURITY_ATTRIBUTES ThreadAttributes;
  ThreadAttributes.bInheritHandle = false;
  ThreadAttributes.lpSecurityDescriptor = NULL;
  ThreadAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);

  hWatchThread = CreateThread(&ThreadAttributes, 0, WatcherThreadInvoker, this, 0, NULL);
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

DWORD Watch::ReadDirectoryChanges() {
  UINT m_nTimerID;
  auto path = pConfig->GetEhndPath() + L"\\Ehnd";

  HANDLE hDir = CreateFile(path.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, 0,
                           OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, 0);
  CONST DWORD cbBuffer = 1024;
  BYTE pBuffer[1024];
  BOOL bWatchSubtree = FALSE;
  DWORD dwNotifyFilter = FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME |
                         FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_SIZE |
                         FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION;
  DWORD bytesReturned;

  m_nTimerID =
    timeSetEvent(500, 0, (LPTIMECALLBACK)Poller, reinterpret_cast<DWORD_PTR>(this), TIME_PERIODIC);

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

    if (bWatch) {
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
      files_ = fileList_Temp;
    }
  }
  return 0;
}

MMRESULT Watch::Callback() {
  using namespace std;

  if (files_.empty()) {
    return 0;
  }

  vector<wstring> file_copies;
  swap(file_copies, files_);

  return callback_(file_copies);
}
