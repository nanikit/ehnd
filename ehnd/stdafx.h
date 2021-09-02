// stdafx.h : 자주 사용하지만 자주 변경되지는 않는
// 표준 시스템 포함 파일 및 프로젝트 관련 포함 파일이
// 들어 있는 포함 파일입니다.
//

#pragma once

typedef struct IUnknown IUnknown;
#include "targetver.h"

#include <Windows.h>

// Additional windows headers
#include <Psapi.h>
#include <Richedit.h>
#include <tchar.h>

constexpr auto EHND_VER = _T("V3.20");

// C++ standard libraries
#include <algorithm>
#include <array>
#include <boost/regex.hpp>
#include <chrono>
#include <cstdarg>
#include <cstdlib>
#include <iostream>
#include <vector>
using namespace std;
using namespace boost;
using namespace chrono;
#define PREFILTER 1
#define POSTFILTER 2

#define NORMAL_LOG 0
#define ERROR_LOG 10
#define DETAIL_LOG 20
#define TIME_LOG 30
#define SKIPLAYER_LOG 40
#define USERDIC_LOG 50

#define USERDIC_COMM 1
#define USERDIC_NOUN 2

// TODO: 프로그램에 필요한 추가 헤더는 여기에서 참조합니다.
#include "config.h"
#include "ehnd.h"
#include "filter.h"
#include "hook.h"
#include "log.h"
#include "watch.h"

extern HINSTANCE g_hInst;
extern filter* pFilter;
extern watch* pWatch;
extern config* pConfig;
extern int g_initTick;
extern char g_DicPath[MAX_PATH];
extern BOOL g_bAnemone;

extern LPBYTE lpfnRetn;
extern LPBYTE lpfnfopen;
extern LPBYTE lpfnwc2mb;
extern int wc2mb_type;
extern LPBYTE lpfnmb2wc;
extern int mb2wc_type;

extern HMODULE hEzt, hMsv;
extern BOOL initOnce;

#define D(x) deformatted_string(x).c_str()