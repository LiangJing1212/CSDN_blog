#include <atomic>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

int main() {
  constexpr int NTHREADS = 4;
  constexpr int INCREMENTS = 100000;

  // ============ 1. 数据竞争：无保护 ============
  {
    int counter = 0;   // 非原子，多线程读写 → 数据竞争 → UB
    std::vector<std::thread> ts;
    for (int t = 0; t < NTHREADS; ++t) {
      ts.emplace_back([&] {
        for (int i = 0; i < INCREMENTS; ++i) {
          int tmp = counter;                 // ① 读
          asm volatile("" ::: "memory");     // 编译器屏障：阻止把读+写合并成单条 inc
          counter = tmp + 1;                 // ② 写（丢掉了中间被别的线程覆盖的 +1）
        }
      });
    }
    for (auto & t : ts) { t.join(); }
    std::cout << "[no-lock]     counter = " << counter
              << "  (期望 " << NTHREADS * INCREMENTS << ")\n";
  }

  // ============ 2. mutex 保护 ============
  {
    int counter = 0;
    std::mutex mtx;
    std::vector<std::thread> ts;
    for (int t = 0; t < NTHREADS; ++t) {
      ts.emplace_back([&] {
        for (int i = 0; i < INCREMENTS; ++i) {
          std::lock_guard<std::mutex> lock(mtx);  // RAII 加锁解锁
          ++counter;
        }
      });
    }
    for (auto & t : ts) { t.join(); }
    std::cout << "[mutex]       counter = " << counter
              << "  (期望 " << NTHREADS * INCREMENTS << ")\n";
  }

  // ============ 3. atomic 无锁递增 ============
  {
    std::atomic<int> counter{0};
    std::vector<std::thread> ts;
    for (int t = 0; t < NTHREADS; ++t) {
      ts.emplace_back([&] {
        for (int i = 0; i < INCREMENTS; ++i) {
          counter.fetch_add(1);   // 原子自增，无锁
        }
      });
    }
    for (auto & t : ts) { t.join(); }
    std::cout << "[atomic]      counter = " << counter.load()
              << "  (期望 " << NTHREADS * INCREMENTS << ")\n";
  }
}
