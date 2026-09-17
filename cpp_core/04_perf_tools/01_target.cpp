#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <vector>

// ============ 分析目标：故意埋 2 个问题 ============
// 1. 热点：低效的平方根循环（可被 sqrt 或查表优化）
// 2. 内存泄漏：new 不 delete（heaptrack/valgrind 抓）

// 低效实现：不用 <cmath> 的 sqrt，暴力迭代逼近
double slow_sqrt(double x, int iters) {
  if (x <= 0) { return 0; }
  double guess = x;
  for (int i = 0; i < iters; ++i) {
    guess = 0.5 * (guess + x / guess);   // 牛顿迭代
  }
  return guess;
}

int main(int argc, char ** argv) {
  int n = 200000;
  if (argc > 1) { n = std::atoi(argv[1]); }

  // 热点块：大量浮点运算
  volatile double acc = 0;
  for (int i = 1; i <= n; ++i) {
    acc += slow_sqrt(static_cast<double>(i), 50);
  }
  std::cout << "acc = " << acc << '\n';

  // 泄漏块：new 不 delete
  for (int i = 0; i < 100; ++i) {
    auto * p = new int[1024];      // ← 泄漏：从不 delete
    p[0] = i;
  }

  // 高频小分配（heaptrack 会看到分配风暴）
  std::vector<int> v;
  for (int i = 0; i < 10000; ++i) {
    v.push_back(i);
  }
  std::cout << "v.size = " << v.size() << '\n';
  return 0;
}
