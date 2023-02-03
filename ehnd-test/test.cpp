#include "pch.h"

#include "../ehnd/ehnd.h"

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
  auto user = "CSUSER123455";
  auto path = std::filesystem::current_path().append("Dat").string();
  auto key = path.c_str();
  J2K_InitializeEx(user, key);
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
