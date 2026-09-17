#include <iostream>
#include <type_traits>

// ============ 编译期 if：偏特化 ============
// 主模板：普通类型
template <typename T>
struct TypeName { static const char * value() { return "other"; } };

// 偏特化：指针类型
template <typename T>
struct TypeName<T*> { static const char * value() { return "pointer"; } };

// 偏特化：const 类型
template <typename T>
struct TypeName<const T> { static const char * value() { return "const"; } };

// 全特化：int
template <>
struct TypeName<int> { static const char * value() { return "int"; } };

// ============ 编译期"循环"：递归 ============
template <unsigned N>
struct Factorial {
  static constexpr unsigned value = N * Factorial<N-1>::value;
};
template <>
struct Factorial<0> { static constexpr unsigned value = 1; };

// ============ type_traits 实战：is_pointer ============
// 手写一个标准库 type_trait 内部长啥样
template <typename T>
struct MyIsPointer : std::false_type {};          // 默认不是指针
template <typename T>
struct MyIsPointer<T*> : std::true_type {};       // 偏特化：指针

int main() {
  std::cout << "TypeName<double>   = " << TypeName<double>::value() << '\n';
  std::cout << "TypeName<int*>     = " << TypeName<int*>::value() << '\n';
  std::cout << "TypeName<const T>  = " << TypeName<const double>::value() << '\n';
  std::cout << "TypeName<int>      = " << TypeName<int>::value() << '\n';

  std::cout << "Factorial<5>       = " << Factorial<5>::value << '\n';

  std::cout << "MyIsPointer<int>   = " << MyIsPointer<int>::value << '\n';
  std::cout << "MyIsPointer<int*>  = " << MyIsPointer<int*>::value << '\n';

  // 编译期断言：错了直接编译失败
  static_assert(MyIsPointer<int*>::value, "int* should be pointer");
  static_assert(!MyIsPointer<int>::value, "int should not be pointer");
  static_assert(Factorial<5>::value == 120, "5! should be 120");
  std::cout << "\nall static_assert passed\n";
}
