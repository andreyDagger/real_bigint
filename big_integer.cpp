#include "big_integer.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <numeric>
#include <ostream>
#include <stdexcept>

const big_integer big_integer::ZERO = 0;
const std::vector<uint32_t> big_integer::TEN_POWERS = {10,      100,      1000,      10000,     100000,
                                                       1000000, 10000000, 100000000, 1000000000};

bool big_integer::is_correct_digit(char ch) noexcept {
  return std::isdigit(static_cast<unsigned int>(ch));
}

void big_integer::remove_leading_zeros(std::vector<uint32_t>& v) noexcept {
  while (!v.empty() && v.back() == 0) {
    v.pop_back();
  }
}

bool big_integer::abs_greater(const big_integer& rhs) noexcept { // |this| > |rhs|
  if (size() > rhs.size()) {
    return true;
  } else if (size() < rhs.size()) {
    return false;
  } else {
    return std::lexicographical_compare(digits.rbegin(), digits.rend(), rhs.digits.rbegin(), rhs.digits.rend(),
                                        std::greater());
  }
}

void big_integer::negate() noexcept {
  if (*this != 0) {
    is_negative ^= 1;
  }
}

void big_integer::apply_additional_code() {
  is_negative = false;
  for (size_t i = 0; i < size(); i++) {
    digits[i] = ~digits[i];
  }
  ++(*this);
}

void big_integer::add_to_ith(size_t pos, uint32_t x) {
  for (size_t i = pos; i < size() && x > 0; ++i) {
    uint64_t result = static_cast<uint64_t>(digits[i]) + x;
    digits[i] = result & MASK;
    x = result >> BASE_LOG2;
  }
  if (x) {
    digits.emplace_back(x);
  }
}

uint32_t big_integer::first_digit() const noexcept {
  if (*this == big_integer::ZERO) {
    return 0;
  } else {
    return digits[0];
  }
}

void big_integer::swap(big_integer& other) noexcept {
  std::swap(is_negative, other.is_negative);
  std::swap(digits, other.digits);
}

size_t big_integer::size() const noexcept {
  return digits.size();
}

big_integer::big_integer(std::vector<uint32_t>& digits, bool is_negative) : digits(digits), is_negative(is_negative) {}

big_integer::big_integer() : digits(), is_negative(false) {}

big_integer::big_integer(const std::string& str) : digits(), is_negative(false) {
  bool negative = false;
  if (str.empty()) {
    throw std::invalid_argument("String initializer must not be empty");
  }
  size_t offset = 0;
  if (str[0] == '-') {
    if (str.length() == 1) {
      throw std::invalid_argument("Wrong string initializer: \"-\"");
    }
    offset++;
    negative = true;
  }
  while (offset < str.size() && str[offset] == '0') {
    offset++;
  }
  if (offset == str.size()) {
    return;
  }
  for (size_t i = offset; i < str.size(); i++) {
    if (!is_correct_digit(str[i])) {
      throw std::invalid_argument("String initializer must contain only digits");
    }
  }

  for (size_t i = offset; i < str.length(); i += TEN_POWERS.size()) {
    *this *= TEN_POWERS[std::min(TEN_POWERS.size() - 1, str.length() - i - 1)];
    uint32_t cur_digit = std::stoi(str.substr(i, TEN_POWERS.size()));
    *this += cur_digit;
  }
  is_negative = negative;
}

big_integer::~big_integer() = default;

big_integer::big_integer(unsigned long long x) : is_negative(false) {
  if (x == 0) {
    return;
  }
  if (x < BASE) {
    digits = {static_cast<uint32_t>(x)};
  } else {
    digits = {static_cast<uint32_t>(x % BASE), static_cast<uint32_t>(x / BASE)};
  }
}

big_integer::big_integer(long long x) : big_integer(my_abs(x)) {
  is_negative = (x < 0);
}

big_integer::big_integer(int x) : big_integer(static_cast<long long>(x)) {}

big_integer::big_integer(unsigned int x) : big_integer(static_cast<unsigned long long>(x)) {}

big_integer::big_integer(unsigned long x) : big_integer(static_cast<unsigned long long>(x)) {}

big_integer::big_integer(long x) : big_integer(static_cast<long long>(x)) {}

uint64_t big_integer::my_abs(int64_t a) {
  if (a != INT64_MIN) {
    return std::abs(a);
  }
  return 1ull << 63;
}

big_integer& big_integer::operator=(const big_integer& other) {
  if (&other != this) {
    big_integer(other).swap(*this);
  }
  return *this;
}

big_integer& big_integer::operator+=(const big_integer& rhs) {
  if (is_negative != rhs.is_negative) {
    negate();
    *this -= rhs;
    negate();
  } else {
    digits.reserve(std::max(size(), rhs.size()) + 1);
    uint8_t carry = 0;
    for (size_t i = 0; i < std::max(size(), rhs.size()); ++i) {
      uint64_t d1 = (i >= size() ? 0 : digits[i]);
      uint64_t d2 = (i >= rhs.size() ? 0 : rhs.digits[i]);
      if (i >= size()) {
        digits.emplace_back((d1 + d2 + carry) & MASK);
      } else {
        digits[i] = (d1 + d2 + carry) & MASK;
      }
      carry = (d1 + d2 + carry) >= big_integer::BASE;
    }
    if (carry == 1) {
      digits.emplace_back(carry);
    }
  }
  return *this;
}

big_integer& big_integer::operator+=(int64_t rhs) {
  if (rhs < 0) {
    return (*this -= -rhs);
  }
  uint8_t carry;
  if (*this >= ZERO) {
    digits.reserve(digits.size() + 1);
    for (size_t i = 0; i < size(); ++i) {
      carry = (static_cast<uint64_t>(digits[i]) + static_cast<uint64_t>(rhs)) >= big_integer::BASE;
      digits[i] += rhs;
      rhs = carry;
    }
    if (rhs > 0) {
      digits.emplace_back(rhs);
    }
  } else {
    if (size() == 1 && digits[0] < rhs) {
      digits[0] = rhs - digits[0];
      is_negative = false;
    } else {
      for (size_t i = 0; i < size(); ++i) {
        carry = static_cast<uint64_t>(digits[i]) < static_cast<uint64_t>(rhs);
        digits[i] -= rhs;
        rhs = carry;
      }
      assert(rhs == 0);
    }
  }
  return *this;
}

big_integer& big_integer::operator-=(const big_integer& rhs) {
  if (is_negative != rhs.is_negative) {
    negate();
    *this += rhs;
    negate();
  } else {
    digits.reserve(std::max(size(), rhs.size()) + 1);
    bool new_sign = (*this < rhs);
    bool greater = abs_greater(rhs);
    uint64_t carry = 0;
    for (size_t i = 0; i < std::max(size(), rhs.size()); ++i) {
      uint64_t d1 = (i >= size() ? 0 : digits[i]);
      uint64_t d2 = (i >= rhs.size() ? 0 : rhs.digits[i]);
      if (greater) {
        if (i >= size()) {
          digits.emplace_back((d1 - d2 - carry + big_integer::BASE) % big_integer::BASE);
        } else {
          digits[i] = (d1 - d2 - carry + big_integer::BASE) % big_integer::BASE;
        }
        carry = (d1 < d2 + carry);
      } else {
        if (i >= size()) {
          digits.emplace_back((d2 - d1 - carry + big_integer::BASE) % big_integer::BASE);
        } else {
          digits[i] = (d2 - d1 - carry + big_integer::BASE) % big_integer::BASE;
        }
        carry = (d2 < d1 + carry);
      }
    }
    is_negative = new_sign;
    assert(carry == 0);
    big_integer::remove_leading_zeros(digits);
  }
  return *this;
}

big_integer& big_integer::operator-=(int64_t rhs) {
  if (rhs == 0) {
    return *this;
  }
  if (rhs < 0) {
    return (*this += -rhs);
  }
  if (is_negative) {
    negate();
    *this += rhs;
    negate();
  } else {
    bool new_sign = (*this < rhs);
    bool greater = abs_greater(rhs);
    for (size_t i = 0; i < size(); ++i) {
      uint64_t d1 = (i >= size() ? 0 : digits[i]);
      if (greater) {
        digits[i] = (d1 - rhs + big_integer::BASE) % big_integer::BASE;
        rhs = (d1 < rhs);
      } else {
        digits[i] = (rhs - d1 + big_integer::BASE) % big_integer::BASE;
        rhs = (rhs < d1);
      }
    }
    is_negative = new_sign;
    assert(rhs == 0);
    big_integer::remove_leading_zeros(digits);
  }
  return *this;
}

big_integer& big_integer::operator*=(const big_integer& rhs) {
  size_t prev_size = size();
  digits.resize(size() + rhs.size());
  for (size_t i = prev_size; i > 0; --i) {
    for (size_t j = rhs.size(); j > 0; --j) {
      uint64_t tmp = static_cast<uint64_t>(digits[i - 1]) * rhs.digits[j - 1];
      if (j > 1) {
        add_to_ith(i - 1 + j - 1, tmp & MASK);
      } else {
        digits[i - 1 + j - 1] = tmp & MASK;
      }
      add_to_ith(i - 1 + j - 1 + 1, tmp >> BASE_LOG2);
    }
  }

  remove_leading_zeros(digits);
  is_negative = (is_negative != rhs.is_negative);

  return *this;
}

big_integer& big_integer::operator*=(int64_t rhs) {
  digits.reserve(digits.size() + 1);
  uint64_t positive_rhs = my_abs(rhs);
  uint64_t cur_digit = 0;
  for (size_t i = 0; i < size(); i++) {
    cur_digit += static_cast<uint64_t>(digits[i]) * positive_rhs;
    digits[i] = cur_digit & MASK;
    cur_digit >>= big_integer::BASE_LOG2;
  }
  assert(cur_digit < big_integer::BASE);
  if (cur_digit > 0) {
    digits.emplace_back(cur_digit);
  }
  if (rhs < 0) {
    negate();
  }
  return *this;
}

big_integer& big_integer::operator/=(const big_integer& rhs) {
  big_integer result = *this / rhs;
  swap(result);
  return *this;
}

big_integer& big_integer::operator%=(const big_integer& rhs) {
  big_integer result = *this % rhs;
  swap(result);
  return *this;
}

bool big_integer::or_negate_predicate(bool is_negative_lhs, bool is_negative_rhs) {
  return is_negative_lhs || is_negative_rhs;
}

bool big_integer::and_negate_predicate(bool is_negative_lhs, bool is_negative_rhs) {
  return is_negative_lhs && is_negative_rhs;
}

bool big_integer::xor_negate_predicate(bool is_negative_lhs, bool is_negative_rhs) {
  return is_negative_lhs ^ is_negative_rhs;
}

uint64_t big_integer::or_operation(uint64_t lhs, uint64_t rhs) {
  return lhs | rhs;
}

uint64_t big_integer::and_operation(uint64_t lhs, uint64_t rhs) {
  return lhs & rhs;
}

uint64_t big_integer::xor_operation(uint64_t lhs, uint64_t rhs) {
  return lhs ^ rhs;
}

void big_integer::do_bitwise_operation(const big_integer& rhs, uint64_t (*operation)(uint64_t, uint64_t),
                                       bool (*negate_predicate)(bool, bool)) {
  digits.reserve(std::max(size(), rhs.size()));
  uint64_t add1 = is_negative, add2 = rhs.is_negative;
  for (size_t i = 0; i < std::max(size(), rhs.size()) || add1 == 1 || add2 == 1; i++) {
    uint64_t d1 = 0, d2 = 0;
    if (i < size()) {
      d1 = digits[i];
    }
    if (i < rhs.size()) {
      d2 = rhs.digits[i];
    }
    if (is_negative) {
      d1 = (~d1) & MASK;
    }
    if (rhs.is_negative) {
      d2 = (~d2) & MASK;
    }

    if (add1 == 1) {
      if (d1 == MASK) {
        d1 = 0;
      } else {
        d1++;
        add1 = 0;
      }
    }

    if (add2 == 1) {
      if (d2 == MASK) {
        d2 = 0;
      } else {
        d2++;
        add2 = 0;
      }
    }

    if (i >= size()) {
      digits.emplace_back(d1 & d2);
    } else {
      digits[i] = operation(d1, d2);
    }
  }
  big_integer::remove_leading_zeros(digits);
  if (negate_predicate(is_negative, rhs.is_negative)) {
    apply_additional_code();
    is_negative = true;
  } else {
    is_negative = false;
  }
}

big_integer& big_integer::operator&=(const big_integer& rhs) {
  do_bitwise_operation(rhs, and_operation, and_negate_predicate);
  return *this;
}

big_integer& big_integer::operator|=(const big_integer& rhs) {
  do_bitwise_operation(rhs, or_operation, or_negate_predicate);
  return *this;
}

big_integer& big_integer::operator^=(const big_integer& rhs) {
  do_bitwise_operation(rhs, xor_operation, xor_negate_predicate);
  return *this;
}

big_integer& big_integer::operator<<=(int rhs) {
  assert(rhs >= 0);
  if (*this == big_integer::ZERO) {
    return *this;
  }
  uint8_t d = rhs % BASE_LOG2;
  int tot = rhs / BASE_LOG2;
  digits.reserve(digits.size() + tot + 1);
  digits.insert(digits.begin(), tot, 0);
  uint32_t prev = 0;
  for (size_t c = 0; c < size(); ++c) {
    uint32_t tmp = (static_cast<uint64_t>(digits[c]) << d) | prev;
    prev = static_cast<uint64_t>(digits[c]) >> (BASE_LOG2 - d);
    digits[c] = tmp;
  }
  digits.emplace_back(prev);
  remove_leading_zeros(digits);
  return *this;
}

big_integer& big_integer::operator>>=(int rhs) {
  assert(rhs >= 0);
  bool sub = false;
  uint64_t d = rhs % BASE_LOG2;
  int tot = rhs / BASE_LOG2;
  for (size_t i = 0; i < std::min(static_cast<int>(size()), tot); ++i) {
    if (digits[i] != 0) {
      sub = true;
    }
  }
  if (tot >= size()) {
    digits.clear();
  } else {
    digits.erase(digits.begin(), digits.begin() + tot);
  }
  sub = (sub | (first_digit() % (1u << d) != 0)) && is_negative; // Очень странное условие на вычет единицы

  *this /= 1u << d;
  if (sub) {
    (*this)--;
  }
  return *this;
}

big_integer big_integer::operator+() const {
  return *this;
}

big_integer big_integer::operator-() const {
  big_integer result = *this;
  if (result != big_integer::ZERO) {
    result.is_negative ^= 1;
  }
  return result;
}

big_integer big_integer::operator~() const {
  big_integer tmp = -(*this);
  tmp -= 1;
  return tmp;
}

big_integer& big_integer::operator++() {
  *this += 1;
  return *this;
}

big_integer big_integer::operator++(int) {
  big_integer result = *this;
  ++(*this);
  return result;
}

big_integer& big_integer::operator--() {
  *this -= 1;
  return *this;
}

big_integer big_integer::operator--(int) {
  big_integer result = *this;
  --(*this);
  return result;
}

big_integer operator+(const big_integer& lhs, const big_integer& rhs) {
  big_integer tmp(lhs);
  tmp += rhs;
  return tmp;
}

big_integer operator+(const big_integer& lhs, int64_t rhs) {
  big_integer tmp(lhs);
  tmp += rhs;
  return tmp;
}

big_integer operator+(int64_t lhs, const big_integer& rhs) {
  big_integer tmp(rhs);
  tmp += lhs;
  return tmp;
}

big_integer operator-(const big_integer& lhs, const big_integer& rhs) {
  big_integer tmp(lhs);
  tmp -= rhs;
  return tmp;
}

big_integer operator-(const big_integer& lhs, int64_t rhs) {
  big_integer tmp(lhs);
  tmp -= rhs;
  return tmp;
}

big_integer operator-(int64_t lhs, const big_integer& rhs) {
  big_integer tmp(rhs);
  tmp -= lhs;
  return tmp;
}

big_integer operator*(const big_integer& lhs, const big_integer& rhs) {
  big_integer tmp(lhs);
  tmp *= rhs;
  return tmp;
}

big_integer operator*(const big_integer& lhs, int64_t rhs) {
  big_integer tmp(lhs);
  tmp *= rhs;
  return tmp;
}

big_integer operator*(int64_t lhs, const big_integer& rhs) {
  big_integer tmp(rhs);
  tmp *= lhs;
  return tmp;
}

std::pair<big_integer, big_integer> div_mod(const big_integer& lhs, uint32_t rhs) {
  if (lhs < rhs) {
    return {0, lhs};
  }
  uint64_t acc = 0;
  std::vector<uint32_t> result;
  result.reserve(lhs.size());
  for (size_t i = lhs.size(); i > 0; i--) {
    acc <<= big_integer::BASE_LOG2;
    acc += lhs.digits[i - 1];
    if (acc >= rhs) {
      uint64_t l = acc / rhs;
      result.emplace_back(l);
      acc -= l * rhs;
    } else {
      result.emplace_back(0);
    }
  }
  std::reverse(result.begin(), result.end());
  big_integer::remove_leading_zeros(result);
  return {big_integer(result), acc};
}

std::pair<big_integer, big_integer> div_mod(const big_integer& lhs, const big_integer& rhs) {
  int k = 0;
  big_integer a = lhs;
  big_integer b = rhs;
  if (a < 0) {
    a.negate();
  }
  if (b < 0) {
    b.negate();
  }
  if (a < b) {
    if (lhs.is_negative) {
      a.negate();
    }
    return {0, a};
  }
  uint32_t last_digit = b.digits.back();
  while (last_digit < big_integer::BASE / 2) {
    ++k;
    last_digit <<= 1;
  }
  a <<= k;
  b <<= k;
  int n = b.size();
  int m = a.size() - n;
  big_integer q;
  q.digits.resize(m + 1);
  if (a >= (b << (big_integer::BASE_LOG2 * m))) {
    q.digits[m] = 1;
    a -= (b << (big_integer::BASE_LOG2 * m));
  } else {
    q.digits[m] = 0;
  }
  for (int j = m - 1; j >= 0; j--) {
    uint64_t d1 = (n + j < a.size() ? a.digits[n + j] : 0);
    uint64_t d2 = (n + j - 1 < a.size() ? a.digits[n + j - 1] : 0);
    q.digits[j] = (d1 * big_integer::BASE + d2) / b.digits[n - 1];
    q.digits[j] = std::min(q.digits[j], static_cast<uint32_t>(big_integer::BASE - 1));
    a -= ((q.digits[j] * b) << (big_integer::BASE_LOG2 * j));
    while (a < 0) {
      q.digits[j]--;
      a += (b << (big_integer::BASE_LOG2 * j));
    }
  }
  big_integer::remove_leading_zeros(q.digits);
  if (lhs.is_negative != rhs.is_negative) {
    q.negate();
  }
  if (lhs.is_negative) {
    a.negate();
  }
  return {q, a >> k};
}

big_integer operator/(const big_integer& lhs, const big_integer& rhs) {
  std::pair<big_integer, big_integer> result = div_mod(lhs, rhs);
  return result.first;
}

big_integer operator%(const big_integer& lhs, const big_integer& rhs) {
  std::pair<big_integer, big_integer> result = div_mod(lhs, rhs);
  return result.second;
}

big_integer operator&(const big_integer& lhs, const big_integer& rhs) {
  big_integer tmp(lhs);
  tmp &= rhs;
  return tmp;
}

big_integer operator|(const big_integer& a, const big_integer& b) {
  big_integer tmp(a);
  tmp |= b;
  return tmp;
}

big_integer operator^(const big_integer& a, const big_integer& b) {
  big_integer tmp(a);
  tmp ^= b;
  return tmp;
}

big_integer operator<<(const big_integer& a, int b) {
  big_integer tmp(a);
  tmp <<= b;
  return tmp;
}

big_integer operator>>(const big_integer& a, int b) {
  big_integer tmp(a);
  tmp >>= b;
  return tmp;
}

bool operator==(const big_integer& a, const big_integer& b) {
  return a.digits == b.digits && a.is_negative == b.is_negative;
}

bool operator!=(const big_integer& a, const big_integer& b) {
  return !(a == b);
}

bool operator<(const big_integer& a, const big_integer& b) {
  if (a.is_negative != b.is_negative) {
    return a.is_negative;
  }
  if (a.size() != b.size()) {
    return (a.size() > b.size()) == a.is_negative;
  }
  if (!a.is_negative) {
    return std::lexicographical_compare(a.digits.rbegin(), a.digits.rend(), b.digits.rbegin(), b.digits.rend());
  } else {
    return std::lexicographical_compare(a.digits.rbegin(), a.digits.rend(), b.digits.rbegin(), b.digits.rend(),
                                        std::greater());
  }
}

bool operator>(const big_integer& a, const big_integer& b) {
  return b < a;
}

bool operator<=(const big_integer& a, const big_integer& b) {
  return !(a > b);
}

bool operator>=(const big_integer& a, const big_integer& b) {
  return !(a < b);
}

std::string to_string(const big_integer& a) {
  if (a == big_integer::ZERO) {
    return "0";
  }
  big_integer b = a;
  std::vector<std::string> digits;
  bool negative = b.is_negative;
  if (b.is_negative) {
    b.negate();
  }
  while (b != big_integer::ZERO) {
    std::pair<big_integer, big_integer> tmp = div_mod(b, big_integer::TEN_POWERS.back());
    b.swap(tmp.first);
    std::string str = std::to_string(tmp.second.first_digit());
    if (b != 0) {
      digits.emplace_back(big_integer::TEN_POWERS.size() - str.size(), '0');
      digits.back() += str;
    } else {
      digits.emplace_back(str);
    }
  }
  std::string result;
  if (negative) {
    result += "-";
  }
  std::reverse(digits.begin(), digits.end());
  for (size_t i = 0; i < digits.size(); ++i) {
    result += digits[i];
  }
  return result;
}

std::ostream& operator<<(std::ostream& out, const big_integer& a) {
  return out << to_string(a);
}