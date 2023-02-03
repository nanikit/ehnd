#pragma once
#include <minwindef.h>

#include "config.h"
#include "ehnd.h"
#include "filter.h"
#include "hook.h"
#include "log.h"
#include "watch.h"

extern HINSTANCE g_hInst;
extern Filter* pFilter;
extern Watch* pWatch;
extern Config* pConfig;
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
