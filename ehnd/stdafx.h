// stdafx.h : 자주 사용하지만 자주 변경되지는 않는
// 표준 시스템 포함 파일 및 프로젝트 관련 포함 파일이
// 들어 있는 포함 파일입니다.

#pragma once

#include "targetver.h"

#include <Windows.h>

#undef min
#undef max

// Additional windows headers
#include <Psapi.h>
#include <Richedit.h>
#include <tchar.h>

// C++ standard libraries
#include <algorithm>
#include <array>
#include <chrono>
#include <filesystem>
#include <format>
#include <fstream>
// cout For debugging
#include <iostream>
#include <mutex>
#include <optional>
#include <random>
#include <ranges>
#include <span>
#include <string>
#include <vector>

// Third parties
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <winreg/WinReg.hpp>
