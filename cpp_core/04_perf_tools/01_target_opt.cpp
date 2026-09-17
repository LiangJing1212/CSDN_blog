#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <vector>

// ============ 优化版：对照 01_target.cpp ============
// 改动：
// 1. slow_sqrt 暴力牛顿迭代 → std::sqrt（硬件指令）
// 2. vector 未 reserve → 预分配，消除 realloc 拷贝

int main(int argc, char ** argv) {
  int n = 200000;
  if (argc > 1) { n = std::atoi(argv[1]); }

  volatile double acc = 0;
  for (int i = 1; i <= n; ++i) {
    acc += std::sqrt(static_cast<double>(i));   // 硬件指令级 sqrt
  }
  std::cout << "acc = " << acc << '\n';

  std::vector<int> v;
  v.reserve(10000);                              // 预分配，避免 10000 次 realloc
  for (int i = 0; i < 10000; ++i) {
    v.push_back(i);
  }
  std::cout << "v.size = " << v.size() << '\n';
  return 0;
}
