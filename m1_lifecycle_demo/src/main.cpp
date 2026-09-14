// ============================================================================
// m1_lifecycle_demo 可执行入口
// ----------------------------------------------------------------------------
// 知识点总览：
//   1. rclcpp::init / shutdown —— ROS2 上下文生命周期
//   2. LifecycleNode 与普通 Node 的 executor 接入差异（get_node_base_interface)
//   3. SingleThreadedExecutor + 独立 spin 线程的工程模式
//   4. 手动驱动状态机：configure → activate → tick → deactivate → cleanup
//   5. spin 线程与主线程的 graceful shutdown 顺序
// ============================================================================

// --- C++ 标准库 -----------------------------------------------------------
// chrono  : 时间间隔（milliseconds 等）
// memory  : std::make_shared / std::shared_ptr
// thread  : std::thread，让 executor 在独立线程 spin，不阻塞主线程的状态机推进
#include <chrono>
#include <memory>
#include <thread>

// --- ROS2 核心 ------------------------------------------------------------
// rclcpp::init(argc, argv) 必须在创建任何 Node 之前调用；
// 它会解析 ROS2 通用参数（如 --ros-args --remap）并初始化全局 context。
#include <rclcpp/rclcpp.hpp>

// 我们自己的 LifecycleNode派生类。
// 注意：LifecycleNode 不是 rclcpp::Node 的子类，而是独立的
// rclcpp_lifecycle::LifecycleNode，因此它不直接兼容 add_node。
#include "m1_lifecycle_demo/lifecycle_node.hpp"

// --------------------------------------------------------------------------
// 入口函数
// --------------------------------------------------------------------------
int main(int argc, char ** argv)
{
  // [知识点 1] 初始化 ROS2 上下文。
  // - argc/argv 让 rclcpp 解析命令行（如 --ros-args -r __ns:=/robot1）
  // - 内部会创建 rclcpp::Context，所有 Node 共享它
  // - 没有这一步，后续 create_node / create_publisher 全部失败
  rclcpp::init(argc, argv);

  // [知识点 2] 创建 LifecycleNode 实例。
  // - NodeOptions 可配置参数：allow_undeclared_parameters、
  //   automatically_declare_parameters_from_overrides、parameter_overrides 等
  // - 此时节点处于 PRIMARY_STATE_UNKNOWN（lifecycle 的起点）
  // - 构造函数只做"无副作用的初始化"，真正的资源申请在 on_configure 回调里
  auto node = std::make_shared<m1_lifecycle_demo::LifecycleDemoNode>(
    rclcpp::NodeOptions());

  // [知识点 3] Executor —— ROS2 的调度器。
  // - SingleThreadedExecutor：一个线程顺序处理所有回调
  // - MultiThreadedExecutor：多线程并发，需配合 callback_group
  // - 这里用单线程即可：demo 不追求吞吐，只演示状态机
  rclcpp::executors::SingleThreadedExecutor exec;

  // [知识点 4] LifecycleNode 接入 executor 的正确姿势。
  // - LifecycleNode 没有直接重载 exec.add_node(node) 的形式
  // - 必须传 node->get_node_base_interface()，让 executor 只看到生命周期受控的接口
  // - 如果传 node 本身，编译都过不了（类型不匹配）
  // - 直观理解：executor 关心的是"有一组回调要调度"，而 lifecycle
  //   只在 ACTIVE 态才希望被调度，所以接入 base interface
  exec.add_node(node->get_node_base_interface());

  // [知识点 5] 把 executor 放到独立线程 spin。
  // - spin() 是阻塞调用，会一直处理回调直到 rclcpp::shutdown()
  // - 如果在主线程 spin，就没法手动调用 node->configure() / activate() 了
  // - 工程模式：spin 线程 <-> 主线程做状态机推进 <-> 最后 shutdown 优雅退出
  // - [&exec] 捕获引用，线程内调 exec.spin()
  std::thread spin_thread([&exec]() {exec.spin();});

  // ---------- 状态机手动驱动开始 ----------
  //
  // lifecycle 的 5 个 primary state：
  //   UNKNOWN(0) → INACTIVE(2) → ACTIVE(4) → INACTIVE(2) → UNCONFIGURED(1)
  //                       ↑________↓              ↑________↓
  //                  configure/activate      deactivate/cleanup
  //
  // 每一步 transition 都会触发我们实现的 on_xxx 回调，里面做对应资源申请/释放。

  // 当前还没 configure，打印初始状态——应当是 "unconfigured"
  RCLCPP_INFO(
    node->get_logger(), "current state: %s",
    node->get_state().label().c_str());

  // [transition 1] UNCONFIGURED → INACTIVE
  // - 触发 on_configure()：里面我们做了 tick_count_=0 + 创建 publisher + 创建 timer
  // - publisher 此刻创建出来但还没激活（LifecyclePublisher 默认 inactive）
  // - timer 已挂上 executor，但因为节点未 ACTIVE，timer 回调不会真正发送消息
  node->configure();
  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  // [transition 2] INACTIVE → ACTIVE
  // - 触发 on_activate()：里面调 tick_pub_->on_activate()，让 publisher 真正可发
  // - 此后 timer 每 200ms 发一次 tick_count_（如果 >0）
  node->activate();
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));

  // 手动 tick 5 次，每次 100ms。
  // - 注意：tick() 只在我们的类里递增计数，不直接 publisher
  // - 真正发消息的是 on_timer()（由 executor 调度）
  // - 所以 ROS topic "lifecycle_tick" 会收到 tick_count 的递增值
  for (int i = 0; i < 5; ++i) {
    node->tick();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  RCLCPP_INFO(node->get_logger(), "tick_count = %zu", node->get_tick_count());

  // [transition 3] ACTIVE → INACTIVE
  // - 触发 on_deactivate()：调 tick_pub_->on_deactivate()，停止发布
  // - 节点回到 INACTIVE，timer 还在但不再发消息
  node->deactivate();
  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  // [transition 4] INACTIVE → UNCONFIGURED
  // - 触发 on_cleanup()：reset timer + reset publisher + tick_count_=0
  // - 资源完全释放，节点回到"未配置"态，可以重新 configure 循环
  node->cleanup();
  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  // ---------- 状态机手动驱动结束 ----------

  // [知识点 6] 优雅退出顺序。
  // 1) rclcpp::shutdown()：通知所有 executor 停止 spin()
  // 2) spin_thread.join()：等 spin 线程真正退出，避免 detached thread 泄漏
  // 3) main 返回 0，进程结束
  // 顺序不能反：join 前必须让 spin 收到 shutdown 信号，否则会死等
  rclcpp::shutdown();
  spin_thread.join();

  return 0;
}
