#include "big_integer.h"
#include "gtest/gtest.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdlib>
#include <limits>
#include <string>
#include <random>

namespace {

class time_limit : public ::testing::Environment {
public:
  void SetUp() override {
    clock_start = std::chrono::steady_clock::now();
  }

  void TearDown() override {
    auto clock_end = std::chrono::steady_clock::now();
    auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(clock_end - clock_start);
    // TODO remove .count() once libc++ implements operator<<(ostream &, const duration &)
    ASSERT_LE(elapsed_time.count(), MAX_ELAPSED_TIME.count());
  }

private:
  static constexpr auto MAX_ELAPSED_TIME = std::chrono::milliseconds(1000);

  std::chrono::steady_clock::time_point clock_start;
};

} // namespace

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
#ifdef ENABLE_TIME_LIMITS
  ::testing::AddGlobalTestEnvironment(new time_limit());
#endif
  return RUN_ALL_TESTS();
}

TEST(correctness, two_plus_two) {
  EXPECT_EQ(big_integer(4), big_integer(2) + big_integer(2));
  EXPECT_EQ(4, big_integer(2) + 2); // implicit conversion from int must work
  EXPECT_EQ(4, 2 + big_integer(2));
}

TEST(correctness, default_ctor) {
  big_integer a;
  big_integer b = 0;
  EXPECT_EQ(0, a);
  EXPECT_EQ(b, a);
}

TEST(correctness, ctor_limits) {
  big_integer a = std::numeric_limits<int>::min();
  big_integer b = std::numeric_limits<int>::max();
  EXPECT_EQ(-1, a + b);
}

TEST(correctness, copy_ctor) {
  big_integer a = 3;
  big_integer b = a;

  EXPECT_EQ(b, a);
  EXPECT_EQ(3, b);
}

TEST(correctness, copy_ctor_real_copy) {
  big_integer a = 3;
  big_integer b = a;
  a = 5;

  EXPECT_EQ(3, b);
}

TEST(correctness, copy_ctor_real_copy2) {
  big_integer a = 3;
  big_integer b = a;
  b = 5;

  EXPECT_EQ(3, a);
}

TEST(correctness, ctor_invalid_string) {
  EXPECT_THROW(big_integer("abc"), std::invalid_argument);
  EXPECT_THROW(big_integer("123x"), std::invalid_argument);
  EXPECT_THROW(big_integer(""), std::invalid_argument);
  EXPECT_THROW(big_integer("-"), std::invalid_argument);
  EXPECT_THROW(big_integer("-x"), std::invalid_argument);
  EXPECT_THROW(big_integer("123-456"), std::invalid_argument);
  EXPECT_THROW(big_integer("--5"), std::invalid_argument);
  EXPECT_THROW(big_integer("++5"), std::invalid_argument);
}

TEST(correctness, assignment_operator) {
  big_integer a = 4;
  big_integer b = 7;
  b = a;

  EXPECT_TRUE(a == b);
}

TEST(correctness, self_assignment) {
  big_integer a = 5;
  a = a;

  EXPECT_TRUE(a == 5);
}

TEST(correctness, assignment_return_value) {
  big_integer a = 4;
  big_integer b = 7;
  (a = b) = a;

  EXPECT_TRUE(a == 7);
  EXPECT_TRUE(b == 7);
}

TEST(correctness, comparisons) {
  big_integer a = 100;
  big_integer b = 100;
  big_integer c = 200;
  big_integer d = -1;

  EXPECT_TRUE(a == b);
  EXPECT_TRUE(a != c);
  EXPECT_TRUE(a < c);
  EXPECT_TRUE(c > a);
  EXPECT_TRUE(a <= a);
  EXPECT_TRUE(a <= b);
  EXPECT_TRUE(a <= c);
  EXPECT_TRUE(c >= a);
  EXPECT_TRUE(d != 0);
}

TEST(correctness, compare_zero_and_minus_zero) {
  big_integer a;
  big_integer b = -a;

  EXPECT_TRUE(a == b);
}

TEST(correctness, add) {
  big_integer a = 5;
  big_integer b = 20;

  EXPECT_TRUE(a + b == 25);

  a += b;
  EXPECT_TRUE(a == 25);
}

TEST(correctness, add_signed) {
  big_integer a = 5;
  big_integer b = -20;
  EXPECT_TRUE(a + b == -15);

  a += b;
  EXPECT_TRUE(a == -15);
}

TEST(correctness, add_return_value) {
  big_integer a = 5;
  big_integer b = 1;

  (a += b) += b;
  EXPECT_EQ(7, a);
}

TEST(correctness, sub) {
  big_integer a = 20;
  big_integer b = 5;

  EXPECT_TRUE(a - b == 15);

  a -= b;
  EXPECT_TRUE(a == 15);
}

TEST(correctness, sub_signed) {
  big_integer a = 5;
  big_integer b = 20;

  EXPECT_TRUE(a - b == -15);

  a -= b;
  EXPECT_TRUE(a == -15);

  a -= -100;
  EXPECT_TRUE(a == 85);
}

TEST(correctness, sub_return_value) {
  big_integer a = 5;
  big_integer b = 1;

  (a -= b) -= b;
  EXPECT_EQ(3, a);
}

TEST(correctness, mul) {
  big_integer a = 5;
  big_integer b = 20;
  EXPECT_TRUE(a * b == 100);

  a *= b;
  EXPECT_TRUE(a == 100);
}

TEST(correctness, mul_signed) {
  big_integer a = -5;
  big_integer b = 20;

  EXPECT_TRUE(a * b == -100);

  a *= b;
  EXPECT_TRUE(a == -100);
}

TEST(correctness, mul_return_value) {
  big_integer a = 5;
  big_integer b = 2;

  (a *= b) *= b;
  EXPECT_EQ(20, a);
}

TEST(correctness, div_) {
  big_integer a = 20;
  big_integer b = 5;
  big_integer c = 20;
  EXPECT_EQ(0, b / c);
  EXPECT_TRUE(a / b == 4);
  EXPECT_TRUE(a % b == 0);

  a /= b;
  EXPECT_TRUE(a == 4);

  c %= b;
  EXPECT_TRUE(c == 0);
}

TEST(correctness, div_int_min) {
  big_integer a = std::numeric_limits<int>::min();
  EXPECT_TRUE((a / a) == (a / std::numeric_limits<int>::min()));
}

TEST(correctness, div_int_min_2) {
  big_integer a = std::numeric_limits<int>::min();
  big_integer b = -1;
  big_integer c = a / b;
  EXPECT_TRUE(c == (a / -1));
  EXPECT_TRUE((c - std::numeric_limits<int>::max()) == 1);
}

TEST(correctness, div_signed) {
  big_integer a = -20;
  big_integer b = 5;

  EXPECT_TRUE(a / b == -4);
  EXPECT_TRUE(a % b == 0);
}

TEST(correctness, div_rounding) {
  big_integer a = 23;
  big_integer b = 5;

  EXPECT_TRUE(a / b == 4);
  EXPECT_TRUE(a % b == 3);
}

TEST(correctness, div_rounding_negative) {
  big_integer a = 23;
  big_integer b = -5;
  big_integer c = -23;
  big_integer d = 5;

  EXPECT_TRUE(a / b == -4);
  EXPECT_TRUE(c / d == -4);
  EXPECT_TRUE(a % b == 3);
  EXPECT_TRUE(c % d == -3);
}

TEST(correctness, div_return_value) {
  big_integer a = 100;
  big_integer b = 2;

  (a /= b) /= b;
  EXPECT_EQ(25, a);
}

TEST(correctness, unary_plus) {
  big_integer a = 123;
  big_integer b = +a;

  EXPECT_TRUE(a == b);

  // this code should not compile:
  // &+a;
}

TEST(correctness, negation) {
  big_integer a = 666;
  big_integer b = -a;

  EXPECT_TRUE(b == -666);
}

TEST(correctness, negation_int_min) {
  big_integer a = std::numeric_limits<int>::min();
  big_integer b = -a;

  EXPECT_EQ(b - 1, std::numeric_limits<int>::max());
}

TEST(correctness, increment) {
  big_integer a = 42;
  big_integer pre = ++a;
  big_integer post = a++;

  EXPECT_EQ(43, pre);
  EXPECT_EQ(43, post);
}

TEST(correctness, decrement) {
  big_integer a = 42;
  big_integer pre = --a;
  big_integer post = a--;

  EXPECT_EQ(41, pre);
  EXPECT_EQ(41, post);
}

TEST(correctness, and_) {
  big_integer a = 0x55;
  big_integer b = 0xaa;

  EXPECT_TRUE((a & b) == 0);
  EXPECT_TRUE((a & 0xcc) == 0x44);
  a &= b;
  EXPECT_TRUE(a == 0);
}

TEST(correctness, and_signed) {
  big_integer a = 0x55;
  big_integer b = 0xaa;
  big_integer x = a & (0xaa - 256);

  EXPECT_TRUE((b & -1) == 0xaa);
  EXPECT_TRUE((a & (0xaa - 256)) == 0);
  EXPECT_TRUE((a & (0xcc - 256)) == 0x44);

  big_integer c = 0x55;
  big_integer d = 0xcc;
  EXPECT_EQ(c & d, big_integer(0x44));
}

TEST(correctness, and_signed2) {
  big_integer a("-18446744073709551615"); // -((1 << 64) - 1)
  big_integer b("147573952589676412929"); // (1 << 67)

  EXPECT_EQ(b, a & b);
}

TEST(correctness, and_return_value) {
  big_integer a = 7;

  (a &= 3) &= 6;
  EXPECT_EQ(2, a);
}

TEST(correctness, or_) {
  big_integer a = 0x55;
  big_integer b = 0xaa;

  EXPECT_TRUE((a | b) == 0xff);
  a |= b;
  EXPECT_TRUE(a == 0xff);

  big_integer c = 0x55;
  big_integer d = 0xcc;
  EXPECT_EQ(c | d, big_integer(0xdd));
}

TEST(correctness, or_signed) {
  big_integer a = 0x55;
  big_integer b = 0xaa;
  EXPECT_TRUE((a | (b - 256)) == -1);
}

TEST(correctness, or_signed2) {
  big_integer a("-36893488147419103232"); // -(1 << 65)
  big_integer b("147573952589676412928"); // (1 << 67)

  EXPECT_EQ(a, a | b);
}

TEST(correctness, or_return_value) {
  big_integer a = 1;

  (a |= 2) |= 4;
  EXPECT_EQ(7, a);
}

TEST(correctness, xor_) {
  big_integer a = 0xaa;
  big_integer b = 0xcc;

  EXPECT_TRUE((a ^ b) == 0x66);

  big_integer c = 0x55;
  big_integer d = 0xcc;
  EXPECT_EQ(c ^ d, big_integer(0x99));
}

TEST(correctness, xor_signed) {
  big_integer a = 0xaa;
  big_integer b = 0xcc;

  EXPECT_TRUE((a ^ (b - 256)) == (0x66 - 256));
}

TEST(correctness, xor_return_value) {
  big_integer a = 1;

  (a ^= 2) ^= 1;
  EXPECT_EQ(2, a);
}

TEST(correctness, not_) {
  big_integer a = 0xaa;
  big_integer b = ~a;
  big_integer c = (-a - 1);
  EXPECT_TRUE(~a == (-a - 1));
}

TEST(correctness, shl_) {
  big_integer a = 23;

  EXPECT_TRUE((a << 5) == 23 * 32);

  a <<= 5;
  EXPECT_TRUE(a == 23 * 32);
}

TEST(correctness, shl_return_value) {
  big_integer a = 1;

  (a <<= 2) <<= 1;
  EXPECT_EQ(8, a);
}

TEST(correctness, shr_) {
  big_integer a = 23;

  EXPECT_EQ(5, a >> 2);

  a >>= 2;
  EXPECT_EQ(5, a);
}

TEST(correctness, shr_31) {
  big_integer a = 65536;
  big_integer b = a * a;

  EXPECT_EQ(2, (a * a) >> 31);
}

TEST(correctness, shr_32) {
  big_integer a = 1;
  EXPECT_EQ(0, a >> 32);
}

TEST(correctness, shr_signed) {
  big_integer a = -1234;

  EXPECT_EQ(-155, a >> 3);

  a >>= 3;
  EXPECT_EQ(-155, a);
}

TEST(correctness, shr_return_value) {
  big_integer a = 64;

  (a >>= 2) >>= 1;
  EXPECT_EQ(8, a);
}

TEST(correctness, add_long) {
  big_integer a("10000000000000000000000000000000000000000000000000000000000000"
                "000000000000000000000000000000");
  big_integer b("100000000000000000000000000000000000000");
  big_integer c("10000000000000000000000000000000000000000000000000000100000000"
                "000000000000000000000000000000");

  EXPECT_EQ(c, a + b);
}

TEST(correctness, add_long_signed) {
  big_integer a("-1000000000000000000000000000000000000000000000000000000000000"
                "000000000000000000000000000000");
  big_integer b("10000000000000000000000000000000000000000000000000000000000000"
                "00000000000000000000000000000");

  EXPECT_EQ(0, a + b);
}

TEST(correctness, add_long_signed2) {
  big_integer a("-1000000000000000000000000000000000000000000000000000000000000"
                "000000000000000000000000000000");
  big_integer b("100000000000000000000000000000000000000");
  big_integer c("-9999999999999999999999999999999999999999999999999999000000000"
                "00000000000000000000000000000");

  EXPECT_EQ(c, a + b);
}

TEST(correctness, add_long_pow2) {
  big_integer a("18446744073709551616");
  big_integer b("-18446744073709551616");
  big_integer c("36893488147419103232");

  EXPECT_EQ(c, a + a);
  EXPECT_EQ(a, b + c);
  EXPECT_EQ(a, c + b);
}

TEST(correctness, sub_long) {
  big_integer a("10000000000000000000000000000000000000000000000000000000000000"
                "000000000000000000000000000000");
  big_integer b("100000000000000000000000000000000000000");
  big_integer c("99999999999999999999999999999999999999999999999999999000000000"
                "00000000000000000000000000000");

  EXPECT_EQ(c, a - b);
}

TEST(correctness, sub_long_pow2) {
  big_integer a("36893488147419103232");
  big_integer b("36893488147419103231");

  EXPECT_EQ(1, a - b);
}

TEST(correctness, mul_long) {
  big_integer a("10000000000000000000000000000000000000000000000000000000000000"
                "000000000000000000000000000000");
  big_integer b("100000000000000000000000000000000000000");
  big_integer c("10000000000000000000000000000000000000000000000000000000000000"
                "000000000000000000000000000000"
                "00000000000000000000000000000000000000");

  EXPECT_EQ(c, a * b);
}

TEST(correctness, mul_long_signed) {
  big_integer a("-1000000000000000000000000000000000000000000000000000000000000"
                "000000000000000000000000000000");
  big_integer b("100000000000000000000000000000000000000");
  big_integer c("-1000000000000000000000000000000000000000000000000000000000000"
                "000000000000000000000000000000"
                "00000000000000000000000000000000000000");

  EXPECT_EQ(c, a * b);
}

TEST(correctness, mul_long_signed2) {
  big_integer a("-100000000000000000000000000");
  big_integer c("100000000000000000000000000"
                "00000000000000000000000000");

  EXPECT_EQ(c, a * a);
}

TEST(correctness, mul_long_pow2) {
  big_integer a("18446744073709551616");
  big_integer b("340282366920938463463374607431768211456");
  big_integer c("11579208923731619542357098500868790785326998466564056403945758"
                "4007913129639936");

  EXPECT_EQ(b, a * a);
  EXPECT_EQ(c, b * b);
}

TEST(correctness, div_0_long) {
  big_integer a;
  big_integer b("100000000000000000000000000000000000000000000000000000000000");
  EXPECT_EQ(a, a / b);
}

TEST(correctness, div_long) {
  big_integer a("10000000000000000000000000000000000000000000000000000000000000"
                "000000000000000000000000000000");
  big_integer b("100000000000000000000000000000000000000");
  big_integer c("100000000000000000000000000000000000000000000000000000");

  EXPECT_EQ(c, a / b);
}

TEST(correctness, div_long_signed) {
  big_integer a("-1000000000000000000000000000000000000000000000000000000000000"
                "0000000000000000000000000000000");
  big_integer b("100000000000000000000000000000000000000");
  big_integer c("-100000000000000000000000000000000000000000000000000000");

  EXPECT_EQ(c, a / b);
}

TEST(correctness, div_long_signed2) {
  big_integer a("-1000000000000000000000000000000000000000000000000000000000000"
                "0000000000000000000000000000000");
  big_integer b("-100000000000000000000000000000000000000");
  big_integer c("100000000000000000000000000000000000000000000000000000");

  EXPECT_EQ(c, a / b);
}

TEST(correctness, negation_long) {
  big_integer a("10000000000000000000000000000000000000000000000000000");
  big_integer c("-10000000000000000000000000000000000000000000000000000");

  EXPECT_EQ(-a, c);
  EXPECT_EQ(a, -c);
}

TEST(correctness, shl_long) {
  EXPECT_EQ(big_integer("1091951238831590836520041079875950759639875963123939936"),
            big_integer("34123476213487213641251283746123461238746123847623123") << 5);

  EXPECT_EQ(big_integer("-104637598388784443044449444577438556334703518260785595038524928"),
            big_integer("-817481237412378461284761285761238721364871236412387461238476") << 7);

  EXPECT_EQ(big_integer("265026033927139132419699023286961165415504134688699829142473"
                        "84891392"),
            big_integer("12341236412857618761234871264871264128736412836643859238479") << 31);
}

TEST(correctness, shr_long) {
  EXPECT_EQ(big_integer("4730073393008085198307104580698364137020387111323398632330851"),
            big_integer("151362348576258726345827346582347652384652387562348756234587245") >> 5);

  EXPECT_EQ(big_integer("1118311528397465815295799577134738919815767762822175104787"),
            big_integer("143143875634875624357862345873246581736418273641238413412741") >> 7);

  EXPECT_EQ(big_integer("-1591563309890326054125627839548891585559049824963"),
            big_integer("-3417856182746231874623148723164812376512852437523846123876") >> 31);
}

TEST(correctness, string_conv) {
  EXPECT_EQ("100", to_string(big_integer("100")));
  EXPECT_EQ("100", to_string(big_integer("0100")));
  EXPECT_EQ("0", to_string(big_integer("0")));
  EXPECT_EQ("0", to_string(big_integer("-0")));
  EXPECT_EQ("-1000000000000000", to_string(big_integer("-1000000000000000")));

  big_integer lim = std::numeric_limits<int32_t>::max();
  EXPECT_EQ("2147483647", to_string(lim));
  lim++;
  EXPECT_EQ("2147483648", to_string(lim));
  lim = std::numeric_limits<int32_t>::min();
  EXPECT_EQ("-2147483648", to_string(lim));
  lim--;
  EXPECT_EQ("-2147483649", to_string(lim));
}

namespace {
template <typename T>
void test_converting_ctor(T value) {
  using std::to_string;

  big_integer bi = value;
  EXPECT_EQ(to_string(value), to_string(bi));
}
} // namespace

TEST(correctness, converting_ctor) {
  using std::numeric_limits;

  test_converting_ctor(numeric_limits<short>::min());
  test_converting_ctor(numeric_limits<short>::max());
  test_converting_ctor(numeric_limits<unsigned short>::min());
  test_converting_ctor(numeric_limits<unsigned short>::max());

  test_converting_ctor(numeric_limits<int>::min());
  test_converting_ctor(numeric_limits<int>::max());
  test_converting_ctor(numeric_limits<unsigned>::min());
  test_converting_ctor(numeric_limits<unsigned>::max());

  test_converting_ctor(numeric_limits<long>::min());
  test_converting_ctor(numeric_limits<long>::max());
  test_converting_ctor(numeric_limits<unsigned long>::min());
  test_converting_ctor(numeric_limits<unsigned long>::max());

  test_converting_ctor(numeric_limits<long long>::min());
  test_converting_ctor(numeric_limits<long long>::max());
  test_converting_ctor(numeric_limits<unsigned long long>::min());
  test_converting_ctor(numeric_limits<unsigned long long>::max());
}

TEST(correctness, converting_ctor2) {
  big_integer a(1);
  big_integer b(1U);
  big_integer c(1L);
  big_integer d(1UL);
  big_integer e(1LL);
  big_integer f(1ULL);

  EXPECT_TRUE(a == b);
  EXPECT_TRUE(a == c);
  EXPECT_TRUE(a == d);
  EXPECT_TRUE(a == e);
  EXPECT_TRUE(a == f);
}

TEST(correctness, converting_ctor3) {
  big_integer a(-1);
  big_integer b(-1L);
  big_integer c(-1LL);

  EXPECT_TRUE(a == b);
  EXPECT_TRUE(a == c);
}

TEST(correctness, converting_ctor4) {
  auto num = static_cast<long long>(-1U) << 32;
  big_integer bignum(num);

  EXPECT_EQ(to_string(bignum), std::to_string(num));
}

TEST(correctness, long_long_mul_mine) {
  big_integer a = big_integer("18446744069414584322"); // 2^32 * 2 + 3
  big_integer b = big_integer("17179869182"); // 2^32*5 + 6
  big_integer c = big_integer("316912649946376885974868164604");
  assert(a * b == c);
}

TEST(correctness, long_mul_eq) {
  big_integer a = 21474836484LL;
  big_integer b = 2;
  assert((a *= b) == 42949672968LL);
}

int64_t shifted_rand() {
  int64_t val = rand() - RAND_MAX / 2;
  if (val != 0) {
    return val;
  } else {
    return 1;
  }
}

TEST(correctness_random, mul_div_randomized) {
  constexpr size_t NUMBER_OF_ITERATIONS = 10;
  for (unsigned itn = 0; itn != NUMBER_OF_ITERATIONS; ++itn) {
    std::vector<int64_t> multipliers;

    for (size_t i = 0; i != NUMBER_OF_ITERATIONS; ++i) {
      multipliers.push_back(shifted_rand());
    }

    big_integer accumulator = 1;

    for (size_t i = 0; i != NUMBER_OF_ITERATIONS; ++i) {
      accumulator *= multipliers[i];
    }

    std::shuffle(multipliers.begin(), multipliers.end(), std::mt19937(std::random_device()()));

    for (size_t i = 1; i != NUMBER_OF_ITERATIONS; ++i) {
      accumulator /= multipliers[i];
    }

    EXPECT_TRUE(accumulator == multipliers[0]);
  }
}

TEST(correctness, lul) {
  std::cout << sizeof(uint64_t) << "\n";
  int a = std::stoi("123x");
  std::cout << a << "\n" << errno;
}

TEST(correctness, my_test) {
  big_integer r = 5;
  big_integer e = 3;
  assert((r *= e) == 15);

  big_integer c = 8;
  big_integer d = c * 3;
  assert(d == 24);

  big_integer a = -123;
  big_integer b = a + 124;
  assert(b == 1);

  for (int i = -5; i <= 5; i++) {
    for (int j = -5; j <= 5; j++) {
      if (j == 0) continue;
      EXPECT_EQ(to_string(big_integer(i) / big_integer(j)), std::to_string(i / j));
      EXPECT_EQ(to_string(big_integer(i) % big_integer(j)), std::to_string(i % j));
    }
  }
//
//  big_integer a = big_integer("4663641195126923906784837407911501911781431778682885373876675405702594111386370850916789755551366679636184265563386688227767987213382618093570798840027902468298828829189912240563086697194341978473444295980267699765634136919978050540793189924762300987979025987503407787114269774120251687306020850690659594648268854188141774388351074075624889837720863623947053705738481655677050775641441301130856216692336450468863170840723093155599811437337316192780074734318489765473883183795853659429101176714996344517794764771903566853543808681314215597722627734364265764129230323966440259323131155272669210775219767209818530097300");
//  int shift = 368;
//  big_integer b = a << shift;
//  big_integer c = big_integer("2803906544008661045632361451328065818467701785841501003577485782340833326154269441646067486331019444869226521725830665257181094697780220140727791100417845102992982384122449797310107602134555704673328377203733232793347209094030548611175919904069800570638524705008286813502924125146086958394803541528226155735388847091055566305923450372193234358705682393384922803366002216646332423572654545538214917952207271145239014481805195590215808819429847667288661253057769928190256116740225418288223197803871204472341188880285733652717339282085381585527429983136749741354939293085337073564435434587521526369095430643619764602543908162978704659540568747815897580564436901352944058718143778061226683132466878254141915360185990377613610188800");
//  assert(to_string(b) == "2803906544008661045632361451328065818467701785841501003577485782340833326154269441646067486331019444869226521725830665257181094697780220140727791100417845102992982384122449797310107602134555704673328377203733232793347209094030548611175919904069800570638524705008286813502924125146086958394803541528226155735388847091055566305923450372193234358705682393384922803366002216646332423572654545538214917952207271145239014481805195590215808819429847667288661253057769928190256116740225418288223197803871204472341188880285733652717339282085381585527429983136749741354939293085337073564435434587521526369095430643619764602543908162978704659540568747815897580564436901352944058718143778061226683132466878254141915360185990377613610188800");
  for (int i = 0; i <= 100; i++) {
//    std::cout << i << std::endl;
    for (int j = 0; j <= 40; j++) {
//      EXPECT_EQ(big_integer(i) << j, ((long long)i) << j);
//      EXPECT_EQ(big_integer(i) >> j, ((long long)i) >> j);
      EXPECT_EQ(to_string(big_integer(i) << j), std::to_string(((long long)i) << j));
      EXPECT_EQ(to_string(big_integer(i) >> j), std::to_string(((long long)i) >> j));
    }
  }
//
//  freopen("input.txt", "r", stdin);
//  for (int i = 0; i < 371; i++) {
//    std::string num;
//    std::cin >> num;
//    std::string expected;
//    std::cin >> expected;
//    int shift;
//    std::cin >> shift;
//    big_integer a = big_integer(num);
//    assert((a << shift) == big_integer(expected));
//  }
}