#include <iostream>
#include <string>
#include <type_traits>
#include <vector>

// ============ SFINAE: Substitution Failure Is Not An Error ============
// 模板实例化失败【不是错误】，只是从重载候选集剔除。
// 这里演示：一个函数对"有 size()"和"没有 size()"的类型分别选择重载。

// ---- 只有 T 有成员 size() 才启用（尾置返回类型触发替换） ----
template <typename T>
auto GetSize(const T & t) -> decltype(t.size()) {
  std::cout << "  [has size()] ";
  return t.size();
}

// ---- 其余类型走这个（C 数组没有 size()） ----
std::size_t GetSize(...) {
  std::cout << "  [fallback]   ";
  return 0;
}

// ============ enable_if：SFINAE 的显式开关 ============
// 整数类型专用版本
template <typename T,
          typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
std::string describe(T) {
  return "integral";
}

// 浮点类型专用版本
template <typename T,
          typename std::enable_if<std::is_floating_point<T>::value, int>::type = 0>
std::string describe(T) {
  return "floating";
}

int main() {
  std::vector<int> v(5);
  int raw = 42;

  std::cout << "GetSize(v):   " << GetSize(v) << '\n';    // 匹配 size() 版本
  std::cout << "GetSize(raw): "; GetSize(raw); std::cout << '\n';  // 匹配 fallback

  std::cout << "describe(1)   = " << describe(1)   << '\n';  // integral
  std::cout << "describe(1.5) = " << describe(1.5) << '\n';  // floating
}
