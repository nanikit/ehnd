#include "pch.h"

#include "../ehnd/config.h"
#include "../ehnd/ehnd.h"
#include "../ehnd/hook.h"

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
  auto key = WideToMultiByte(wpath, CP_ACP, true);
  J2K_InitializeEx(user, key.c_str());
}

TEST(TranslationTest, TranslateBasic) {
  Initialize();

  auto result = Translate(simpleJapanese);
  ASSERT_STRNE(L"", result.c_str());
}

TEST(TranslationTest, TranslateHdor) {
  Initialize();

  auto result = Translate(L"乱数生成");
  ASSERT_STREQ(L"난수 생성", result.c_str());
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

void TestTranslationWithSnapshot(std::wistream& original, std::wistream& expected,
                                 std::wostream& actual) {
  using namespace std;

  wstring original_buffer;
  wstring expected_buffer;
  wstring line;
  for (int i = 0;; i++) {
    getline(original, line);
    original_buffer += line;
    original_buffer += L'\n';
    getline(expected, line);
    expected_buffer += line;
    expected_buffer += L'\n';

    if (i % 50 == 49) {
      auto translated = Translate(original_buffer.c_str());
      actual << translated;
      // ASSERT_STREQ(translated.c_str(), expected_buffer.c_str());

      original_buffer.clear();
      expected_buffer.clear();
    }

    ASSERT_EQ(!original, !expected);
    if (!original) {
      break;
    }
  }
}

void TestTranslationWithFileSnapshot() {
}

TEST(TranslationTest, HdorExhaustive) {
  using namespace std;
  using namespace filesystem;

  Initialize();

  auto utf8 = boost::locale::generator().generate("UTF-8");
  auto original = wifstream{L"..\\ehnd-test\\hdor\\PreFilter_@Hdor.txt"};
  auto expected = wifstream{L"..\\ehnd-test\\hdor_expected\\PreFilter_@Hdor.txt"};
  create_directory(L"hdor_actual");
  auto actual = wofstream{L"hdor_actual\\PreFilter_@Hdor.txt"};
  if (!original) {
    FAIL() << "original open failed.";
  }
  if (!expected) {
    FAIL() << "expected open failed.";
  }
  if (!actual) {
    FAIL() << "actual open failed.";
  }
  actual.imbue(utf8);
  original.imbue(utf8);
  expected.imbue(utf8);

  TestTranslationWithSnapshot(original, expected, actual);
}
