#include <iostream>
#include <string>

// ============ 变参模板 + 折叠表达式（C++17） ============

// 折叠表达式：一行算任意个数之和（空包返回 0）
template <typename... Args>
int sum(Args... args) {
  return (args + ... + 0);   // 右折叠，初值 0 防空包
}

// 折叠输出：逗号运算符逐个打印
template <typename... Args>
void print_all(Args... args) {
  (std::cout << ... << args) << '\n';   // 二元左折叠
}

// 递归展开（C++11 老写法，对比理解折叠多省事）
void print_old() { std::cout << '\n'; }   // 递归终点
template <typename T, typename... Rest>
void print_old(T first, Rest... rest) {
  std::cout << first << ' ';
  print_old(rest...);   // 每次吃掉一个参数
}

// 折叠 + sizeof...：包里有几个参数
template <typename... Args>
void count(Args... args) {
  std::cout << "sizeof...(args) = " << sizeof...(args) << '\n';
}

int main() {
  std::cout << "sum(1,2,3,4,5)        = " << sum(1, 2, 3, 4, 5) << '\n';
  std::cout << "sum()                 = " << sum() << '\n';
  std::cout << "sum(1.5, 2.5, 3.0)    = " << sum(1.5, 2.5, 3.0) << '\n';

  std::cout << "print_all(\"a\", 1, 2.5): ";
  print_all("a", 1, 2.5);

  std::cout << "print_old 递归版本: ";
  print_old(10, 20, 30);

  count(1, 2, 3);
}
