﻿// stdafx.h : 자주 사용하지만 자주 변경되지는 않는
// 표준 시스템 포함 파일 및 프로젝트 관련 포함 파일이
// 들어 있는 포함 파일입니다.

#pragma once

typedef struct IUnknown IUnknown;
#include "targetver.h"

#include <Windows.h>

// Additional windows headers
#include <Psapi.h>
#include <Richedit.h>
#include <tchar.h>

// C++ standard libraries
#include <algorithm>
#include <array>
#include <boost/regex.hpp>
#include <chrono>
#include <cstdarg>
#include <cstdlib>
#include <iostream>
#include <vector>

#define D(x) deformatted_string(x).c_str()