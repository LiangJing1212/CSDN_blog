#include <iostream>
#include <string>
#include <utility>

// 三个重载：区分"传左值 / 传右值 / 传 const 左值"
void sink(std::string &)       { std::cout << "  lvalue ref\n"; }
void sink(const std::string &) { std::cout << "  const lvalue ref\n"; }
void sink(std::string &&)      { std::cout << "  rvalue ref\n"; }

// 万能引用：T&& 在模板推导里能匹配任何值类别
template <typename T>
void forward_demo(T && arg) {
  // std::forward<T> 还原 arg 原本的值类别，再往下传
  sink(std::forward<T>(arg));
}

int main() {
  std::string s = "hello";

  std::cout << "传左值 s:\n";
  forward_demo(s);               // T = std::string&,  arg 是左值

  std::cout << "传 const 左值 cs:\n";
  const std::string cs = "world";
  forward_demo(cs);              // T = const std::string&

  std::cout << "传右值 std::move(s):\n";
  forward_demo(std::move(s));    // T = std::string,  arg 是右值

  std::cout << "传临时对象:\n";
  forward_demo(std::string("tmp"));  // T = std::string
}
