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
extern unsigned long long init_tick;
extern BOOL g_bAnemone;

extern HMODULE hEzt, hMsv;
