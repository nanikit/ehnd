#pragma once
#include <Windows.h>
#include <tchar.h>

#include <string>
#include <vector>

extern bool g_bAnemone;

enum class user_word {
  common = 1,
  noun = 2,
};

enum class rule_type {
  preprocess = 1,
  postprocess = 2,
};

struct FILTERSTRUCT {
  int g_line;
  int line;
  std::wstring src;
  std::wstring dest;
  int layer;
  int regex;
  std::wstring db;

  int _ecount;
  double _etime;

  int operator<(const FILTERSTRUCT& fs) const {
    return (layer) < (fs.layer) || ((layer) == (fs.layer) && (g_line < fs.g_line));
  }
};

struct USERDICSTRUCT {
  int g_line;
  int line;
  user_word _type;
  wchar_t _jpn[31];
  wchar_t _kor[31];
  wchar_t _attr[37];
  wchar_t _db[260];
};
int operator<(USERDICSTRUCT& left, USERDICSTRUCT& right);

struct SKIPLAYERSTRUCT {
  std::wstring wtype;
  rule_type type;
  int layer;
  int g_line;
  int line;
  std::wstring wlayer;
  std::wstring cond;
  int operator<(SKIPLAYERSTRUCT ss) {
    return (layer) < (ss.layer) || ((layer) == (ss.layer) && (g_line < ss.g_line));
  }
};

class Filter {
 public:
  Filter();
  ~Filter();

  bool load();
  bool load_dic();

  bool pre_load();
  bool post_load();
  bool userdic_load();
  bool jkdic_load(int& g_line);
  bool anedic_load(int& g_line);
  bool skiplayer_load();
  bool ehnddic_cleanup();
  bool ehnddic_create();
  bool pre(std::wstring& wsText);
  bool post(std::wstring& wsText);
  bool cmd(std::wstring& wsText);

  const wchar_t* GetDicDB(int idx) {
    return UserDic[idx]._db;
  }
  const int GetDicLine(int idx) {
    return UserDic[idx].line;
  }
  const wchar_t* GetDicJPN(int idx) {
    return UserDic[idx]._jpn;
  }
  const wchar_t* GetDicKOR(int idx) {
    return UserDic[idx]._kor;
  }
  const wchar_t* GetDicTYPE(int idx) {
    return (UserDic[idx]._type == user_word::noun
              ? L"명사"
              : (UserDic[idx]._type == user_word::common ? L"상용어구" : L"Unknown"));
  }
  const wchar_t* GetDicATTR(int idx) {
    return UserDic[idx]._attr;
  }

 private:
  bool skiplayer_load2(std::vector<SKIPLAYERSTRUCT>& SkipLayer, LPCWSTR lpPath, LPCWSTR lpFileName,
                       int& g_line);
  bool filter_load(std::vector<FILTERSTRUCT>& Filter, LPCWSTR lpPath, LPCWSTR lpFileName,
                   rule_type rule_type, int& g_line);
  bool userdic_load2(LPCWSTR lpPath, LPCWSTR lpFileName, int& g_line);
  bool filter_proc(std::vector<FILTERSTRUCT>& Filter, rule_type rule_type, std::wstring& wsText);

  std::vector<FILTERSTRUCT> PreFilter;
  std::vector<FILTERSTRUCT> PostFilter;
  std::vector<USERDICSTRUCT> UserDic;
  std::vector<SKIPLAYERSTRUCT> SkipLayer;

  HANDLE hLoadEvent;
};
