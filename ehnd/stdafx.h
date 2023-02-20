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
