#pragma once

#include <concepts>
#include <cstdint>
#include <iosfwd>
#include <iostream>
#include <limits>
#include <string>
#include <type_traits>
#include <vector>

struct big_integer {
private:
  using size_t = std::size_t;
  using uint64_t = std::uint64_t;
  using int64_t = std::int64_t;
  using int32_t = std::int32_t;
  using uint32_t = std::uint32_t;
  using uint8_t = std::uint8_t;

private:
  static const std::vector<uint32_t> TEN_POWERS;
  static const big_integer TEN;
  static const big_integer ZERO;
  static constexpr uint32_t MASK = ((1ll << 32) - 1);
  static constexpr uint8_t BASE_LOG2 = 32;
  static constexpr uint64_t BASE = (1ll << 32);

private:
  std::vector<uint32_t> digits;
  bool is_negative;

  explicit big_integer(std::vector<uint32_t>& digits, bool is_negative = false);

private:
  static bool is_correct_digit(char ch) noexcept;
  static void remove_leading_zeros(std::vector<uint32_t>& v) noexcept;

  bool abs_greater(const big_integer& rhs) noexcept;
  void negate() noexcept;
  void apply_additional_code();
  void add_to_ith(size_t pos, uint32_t x);
  uint32_t first_digit() const noexcept;
  void swap(big_integer& other) noexcept;

  static bool or_negate_predicate(bool is_negative_lhs, bool is_negative_rhs);
  static bool and_negate_predicate(bool is_negative_lhs, bool is_negative_rhs);
  static bool xor_negate_predicate(bool is_negative_lhs, bool is_negative_rhs);
  static uint64_t or_operation(uint64_t lhs, uint64_t rhs);
  static uint64_t and_operation(uint64_t lhs, uint64_t rhs);
  static uint64_t xor_operation(uint64_t lhs, uint64_t rhs);
  void do_bitwise_operation(const big_integer& rhs, uint64_t (*operation)(uint64_t, uint64_t),
                            bool (*negate_predicate)(bool negative1, bool negative2));

  friend std::pair<big_integer, big_integer> div_mod(const big_integer& lhs, const big_integer& rhs);
  friend std::pair<big_integer, big_integer> div_mod(const big_integer& lhs, uint32_t rhs);

  size_t size() const noexcept;

public:
  big_integer();
  big_integer(const big_integer& other) = default;

  big_integer(unsigned long long x);

  big_integer(long long x);

  big_integer(int x);

  big_integer(unsigned int x);

  big_integer(unsigned long x);

  big_integer(long x);

  static uint64_t my_abs(int64_t a);

  explicit big_integer(const std::string& str);
  ~big_integer();

  big_integer& operator=(const big_integer& other);

  big_integer& operator+=(const big_integer& rhs);
  big_integer& operator+=(int64_t rhs);
  big_integer& operator-=(const big_integer& rhs);
  big_integer& operator-=(int64_t rhs);
  big_integer& operator*=(const big_integer& rhs);
  big_integer& operator*=(int64_t rhs);
  big_integer& operator/=(const big_integer& rhs);
  big_integer& operator%=(const big_integer& rhs);

  big_integer& operator&=(const big_integer& rhs);
  big_integer& operator|=(const big_integer& rhs);
  big_integer& operator^=(const big_integer& rhs);

  big_integer& operator<<=(int rhs);
  big_integer& operator>>=(int rhs);

  big_integer operator+() const;
  big_integer operator-() const;
  big_integer operator~() const;

  big_integer& operator++();
  big_integer operator++(int);

  big_integer& operator--();
  big_integer operator--(int);

  friend bool operator==(const big_integer& a, const big_integer& b);
  friend bool operator!=(const big_integer& a, const big_integer& b);
  friend bool operator<(const big_integer& a, const big_integer& b);
  friend bool operator>(const big_integer& a, const big_integer& b);
  friend bool operator<=(const big_integer& a, const big_integer& b);
  friend bool operator>=(const big_integer& a, const big_integer& b);

  friend big_integer operator+(const big_integer& a, const big_integer& b);
  friend big_integer operator-(const big_integer& a, const big_integer& b);
  friend big_integer operator*(const big_integer& a, const big_integer& b);
  friend big_integer operator+(const big_integer& a, int64_t b);
  friend big_integer operator-(const big_integer& a, int64_t b);
  friend big_integer operator*(const big_integer& a, int64_t b);
  friend big_integer operator+(int64_t a, const big_integer& b);
  friend big_integer operator-(int64_t a, const big_integer& b);
  friend big_integer operator*(int64_t a, const big_integer& b);
  friend big_integer operator/(const big_integer& a, const big_integer& b);
  friend big_integer operator%(const big_integer& a, const big_integer& b);

  friend big_integer operator&(const big_integer& a, const big_integer& b);
  friend big_integer operator|(const big_integer& a, const big_integer& b);
  friend big_integer operator^(const big_integer& a, const big_integer& b);

  friend big_integer operator<<(const big_integer& a, int b);
  friend big_integer operator>>(const big_integer& a, int b);

  friend std::ostream& operator<<(std::ostream& out, const big_integer& a);

  friend std::string to_string(const big_integer& a);
};
