#pragma once

struct FILTERSTRUCT
{
	int g_line;
	int line;
	wstring src;
	wstring dest;
	int layer;
	int regex;
	wstring db;
	int operator<(FILTERSTRUCT fs) { return (layer) < (fs.layer) || ((layer) == (fs.layer) && (g_line < fs.g_line)); }
	int _ecount;
	double _etime;
};

struct USERDICSTRUCT
{
	int g_line;
	int line;
	wchar_t _type;
	wchar_t _jpn[31];
	wchar_t _kor[31];
	wchar_t _attr[37];
	wchar_t _db[260];
};
int operator <(USERDICSTRUCT& left, USERDICSTRUCT& right);

struct SKIPLAYERSTRUCT
{
	wstring wtype;
	int type;
	int layer;
	int g_line;
	int line;
	wstring wlayer;
	wstring cond;
	int operator<(SKIPLAYERSTRUCT ss) { return (layer) < (ss.layer) || ((layer) == (ss.layer) && (g_line < ss.g_line)); }
};
class filter
{
public:
	filter();
	~filter();

	bool load();
	bool load_dic();

	bool pre_load();
	bool post_load();
	bool userdic_load();
	bool jkdic_load(int &g_line);
	bool anedic_load(int &g_line);
	bool skiplayer_load();
	bool ehnddic_cleanup();
	bool ehnddic_create();
	bool pre(wstring &wsText);
	bool post(wstring &wsText);
	bool cmd(wstring &wsText);

	const wchar_t *GetDicDB(int idx) { return UserDic[idx]._db; }
	const int GetDicLine(int idx) { return UserDic[idx].line; }
	const wchar_t *GetDicJPN(int idx) { return UserDic[idx]._jpn; }
	const wchar_t *GetDicKOR(int idx) { return UserDic[idx]._kor; }
	const wchar_t *GetDicTYPE(int idx) { return (UserDic[idx]._type == USERDIC_NOUN ? L"명사" : (UserDic[idx]._type == USERDIC_COMM ? L"상용어구" : L"Unknown")); }
	const wchar_t *GetDicATTR(int idx) { return UserDic[idx]._attr; }

private:
	bool skiplayer_load2(vector<SKIPLAYERSTRUCT> &SkipLayer, LPCWSTR lpPath, LPCWSTR lpFileName, int &g_line);
	bool filter_load(vector<FILTERSTRUCT> &Filter, LPCWSTR lpPath, LPCWSTR lpFileName, int FilterType, int &g_line);
	bool userdic_load2(LPCWSTR lpPath, LPCWSTR lpFileName, int &g_line);
	bool filter_proc(vector<FILTERSTRUCT> &Filter, const int FilterType, wstring &wsText);

	vector<FILTERSTRUCT> PreFilter;
	vector<FILTERSTRUCT> PostFilter;
	vector<USERDICSTRUCT> UserDic;
	vector<SKIPLAYERSTRUCT> SkipLayer;

	HANDLE hLoadEvent;
};