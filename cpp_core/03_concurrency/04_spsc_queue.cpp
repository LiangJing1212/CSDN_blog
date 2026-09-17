#include <atomic>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <thread>
#include <vector>

// ============ 无锁 SPSC 环形队列 ============
// SPSC: Single Producer Single Consumer（单生产者单消费者）
// 只有一个读头、一个写尾，靠原子序号避免锁。
// 对应 ROS2 中"一个 producer 线程投递、一个 consumer 线程消费"的解耦。

template <typename T, std::size_t CAP>
class SPSCQueue {
public:
  explicit SPSCQueue() : buffer_(CAP) {}

  // 生产者调用
  bool push(const T & v) {
    const auto head = head_.load(std::memory_order_relaxed);
    const auto next = (head + 1) % CAP;
    if (next == tail_.load(std::memory_order_acquire)) {
      return false;   // 满
    }
    buffer_[head] = v;   // 写数据
    head_.store(next, std::memory_order_release);  // 发布：数据对消费者可见
    return true;
  }

  // 消费者调用
  bool pop(T & out) {
    const auto tail = tail_.load(std::memory_order_relaxed);
    if (tail == head_.load(std::memory_order_acquire)) {
      return false;   // 空
    }
    out = buffer_[tail];    // 读数据
    tail_.store((tail + 1) % CAP, std::memory_order_release);  // 释放槽位
    return true;
  }

private:
  std::vector<T> buffer_;
  std::atomic<std::size_t> head_{0};  // 生产者写位置
  std::atomic<std::size_t> tail_{0};  // 消费者读位置
};

int main() {
  SPSCQueue<int, 1024> q;
  constexpr int N = 1000000;
  std::atomic<int> received{0};

  std::thread producer([&] {
    for (int i = 0; i < N; ++i) {
      while (!q.push(i)) { /* 满则自旋 */ }
    }
  });
  std::thread consumer([&] {
    int last = -1;
    int v;
    while (received.load(std::memory_order_relaxed) < N) {
      if (q.pop(v)) {
        assert(v == last + 1);   // 无锁队列必须无重复无丢失且保序
        last = v;
        received.fetch_add(1, std::memory_order_relaxed);
      }
    }
  });

  producer.join();
  consumer.join();
  std::cout << "SPSC queue: received " << received.load()
            << " / " << N << " in order, no locks used\n";
}
