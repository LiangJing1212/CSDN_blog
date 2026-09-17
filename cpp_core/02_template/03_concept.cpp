#include <concepts>
#include <iostream>
#include <string>
#include <type_traits>

// ============ C++20 Concept：约束即文档 ============
// concept = 编译期约束，直接写在模板参数上

// 定义：可自增的类型
template <typename T>
concept Incrementable = requires(T x) { ++x; };

// 定义：可比较大小的类型（两个约束用 && 组合）
template <typename T>
concept Ordered = requires(T a, T b) {
  { a < b } -> std::convertible_to<bool>;
};

// 用 concept 约束模板参数（替代 enable_if，可读性爆表）
template <Incrementable T>
T step(T v) { return ++v; }

// requires 子句形式（等价的另一种写法）
template <typename T>
  requires Ordered<T>
bool is_less(T a, T b) { return a < b; }

// concept 约束 + 结合 if constexpr 做编译期分派
template <typename T>
std::string categorize(T) {
  if constexpr (std::is_integral_v<T>) {
    return "integral";
  } else if constexpr (std::is_floating_point_v<T>) {
    return "floating";
  } else {
    return "other";
  }
}

int main() {
  std::cout << "step(41)          = " << step(41) << '\n';
  std::cout << "is_less(1, 2)     = " << is_less(1, 2) << '\n';
  std::cout << "is_less(2.5, 1.5) = " << is_less(2.5, 1.5) << '\n';

  std::cout << "categorize(1)     = " << categorize(1)     << '\n';
  std::cout << "categorize(1.5)   = " << categorize(1.5)   << '\n';
  std::cout << "categorize('c')   = " << categorize('c')   << '\n';
}
