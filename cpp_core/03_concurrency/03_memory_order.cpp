#include <atomic>
#include <cassert>
#include <iostream>
#include <thread>
#include <vector>

// ============ 消息传递模式 ============
// 生产者：先写 data，再置 ready 标志
// 消费者：等 ready 置位，再读 data
//
// 关键：data 写入必须对消费者可见，且 data 的写必须发生在 ready 置位【之前】。
// 这靠 release/acquire 配对建立 happens-before。

constexpr int RUNS = 100000;

// ---- 用 release/acquire：正确 ----
void test_acquire_release() {
  for (int r = 0; r < RUNS; ++r) {
    std::atomic<bool> ready{false};
    int data = 0;

    std::thread producer([&] {
      data = 42;                    // 普通写
      ready.store(true, std::memory_order_release);  // release：前面的写全部可见
    });
    std::thread consumer([&] {
      while (!ready.load(std::memory_order_acquire)) {}  // acquire：能读到 ready 就同步
      assert(data == 42);
    });
    producer.join();
    consumer.join();
  }
  std::cout << "acquire/release: " << RUNS << " runs passed\n";
}

// ---- 用 relaxed：顺序不保证 → 模式可能坏（x86 上碰巧对，ARM 上会错）----
void test_relaxed() {
  int failures = 0;
  for (int r = 0; r < RUNS; ++r) {
    std::atomic<bool> ready{false};
    int data = 0;

    std::thread producer([&] {
      data = 42;                    // 普通写
      ready.store(true, std::memory_order_relaxed);  // relaxed：仅原子，无顺序
    });
    std::thread consumer([&] {
      while (!ready.load(std::memory_order_relaxed)) {}
      if (data != 42) { ++failures; }  // 可能读到旧 data（编译器/CPU 重排）
    });
    producer.join();
    consumer.join();
  }
  std::cout << "relaxed: " << failures << " failures in " << RUNS
            << " runs (x86 碰巧 0，ARM 可能非 0)\n";
}

// ---- 全用 seq_cst（默认）：最保守，全序 ----
void test_seq_cst() {
  for (int r = 0; r < RUNS; ++r) {
    std::atomic<bool> ready{false};
    int data = 0;
    std::thread producer([&] {
      data = 42;
      ready.store(true);            // 默认 memory_order_seq_cst
    });
    std::thread consumer([&] {
      while (!ready.load()) {}
      assert(data == 42);
    });
    producer.join();
    consumer.join();
  }
  std::cout << "seq_cst: " << RUNS << " runs passed\n";
}

int main() {
  test_acquire_release();
  test_relaxed();
  test_seq_cst();
}
