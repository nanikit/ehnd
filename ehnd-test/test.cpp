#include "pch.h"

#include "../ehnd/config.h"
#include "../ehnd/ehnd.h"
#include "../ehnd/hook.h"

std::string WideToMultiByte(const std::wstring_view&& source, UINT codePage) {
  using namespace std;

  int i_len =
    WideCharToMultiByte(codePage, 0, source.data(), source.size(), nullptr, 0, nullptr, nullptr);
  string dest;
  dest.resize(i_len);

  const char replacementChar = 0x01;
  BOOL hasUnconvertible = false;
  WideCharToMultiByte(codePage, 0, source.data(), -1, dest.data(), dest.size(), &replacementChar,
                      &hasUnconvertible);
  if (hasUnconvertible) {
    for (auto& ch : dest) {
      if (ch == replacementChar) {
        ch = ' ';
      }
    }
  }

  return dest;
}

std::wstring Translate(const wchar_t* szJpn) {
  using namespace std;

  auto memory = J2K_TranslateMMNTW(0, szJpn);
  auto translated = wstring{memory};
  J2K_FreeMem(memory);

  return translated;
}

auto simpleJapanese =
  L"シカトしてやろうかとも思ったが、どうしても一言文句を言ってやりたいという氣持ちが勝ってしまい電"
  L"話に出る。";

void Initialize() {
  using namespace std;

  auto user = "CSUSER123455";
  auto wpath = Config{}.GetEztransPath() + L"\\Dat";
  auto key = WideToMultiByte(wpath, CP_ACP);
  J2K_InitializeEx(user, key.c_str());
}

TEST(TranslationTest, TranslateBasic) {
  Initialize();

  auto result = Translate(simpleJapanese);
  ASSERT_STRNE(L"", result.c_str());
}

TEST(TranslationTest, EnsureThreadSafety) {
  using namespace std;

  Initialize();

  auto futures = array<future<wstring>, 4>{
    async(Translate, simpleJapanese),
    async(Translate, simpleJapanese),
    async(Translate, simpleJapanese),
    async(Translate, simpleJapanese),
  };

  vector<wstring> results;
  for (auto& fut : futures) {
    results.push_back(fut.get());
  }

  ASSERT_FALSE(results[0].empty());

  auto first_error = adjacent_find(begin(results), end(results), not_equal_to<>());
  ASSERT_EQ(first_error, end(results));
}
