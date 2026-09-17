#include <iostream>
#include <thread>
#include <vector>

void worker(int id, int rounds) {
  for (int i = 0; i < rounds; ++i) {
    // 纯计算，模拟耗时
    volatile double x = 0;
    for (int k = 0; k < 1000; ++k) { x += k; }
  }
  std::cout << "worker " << id << " done\n";
}

int main() {
  constexpr int N = 4;

  std::cout << "== join：等待所有线程完成 ==\n";
  {
    std::vector<std::thread> threads;
    for (int i = 0; i < N; ++i) {
      threads.emplace_back(worker, i, 2000);
    }
    for (auto & t : threads) {
      t.join();   // 阻塞直到该线程结束
    }
    std::cout << "main: all joined\n";
  }

  std::cout << "\n== detach：线程后台跑，主线程不等待 ==\n";
  {
    std::thread detached(worker, 99, 100);
    detached.detach();   // 分离，无法再 join
    std::cout << "main: detached, continuing immediately\n";
  }
  // 注意：detach 的线程若访问了 main 栈上变量，main 退出即悬垂 —— 下个 demo 细讲
}
