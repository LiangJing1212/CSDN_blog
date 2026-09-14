#ifndef M1_LIFECYCLE_DEMO__LIFECYCLE_NODE_HPP_
#define M1_LIFECYCLE_DEMO__LIFECYCLE_NODE_HPP_

#include <cstddef>

#include <rclcpp/rclcpp.hpp>
#include <rclcpp_lifecycle/lifecycle_node.hpp>
#include <rclcpp_lifecycle/lifecycle_publisher.hpp>
#include <rclcpp_lifecycle/managed_entity.hpp>
#include <rclcpp_lifecycle/state.hpp>
#include <rclcpp_lifecycle/transition.hpp>

#include <std_msgs/msg/u_int64.hpp>

namespace m1_lifecycle_demo
{

class LifecycleDemoNode : public rclcpp_lifecycle::LifecycleNode
{
public:
  explicit LifecycleDemoNode(const rclcpp::NodeOptions & options = rclcpp::NodeOptions());

  rclcpp_lifecycle::State get_state();

  void tick();
  std::size_t get_tick_count() const {return tick_count_;}

  // 测试钩子：置 true 后 on_configure 返回 FAILURE，模拟配置失败路径
  void set_fail_on_configure(bool flag) {fail_on_configure_ = flag;}

  using CallbackReturn =
    rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn;

protected:
  CallbackReturn on_configure(const rclcpp_lifecycle::State & state) override;
  CallbackReturn on_activate(const rclcpp_lifecycle::State & state) override;
  CallbackReturn on_deactivate(const rclcpp_lifecycle::State & state) override;
  CallbackReturn on_cleanup(const rclcpp_lifecycle::State & state) override;
  CallbackReturn on_shutdown(const rclcpp_lifecycle::State & state) override;

private:
  bool fail_on_configure_{false};
  std::size_t tick_count_{0};
  rclcpp::TimerBase::SharedPtr timer_;
  rclcpp_lifecycle::LifecyclePublisher<std_msgs::msg::UInt64>::SharedPtr tick_pub_;

  void on_timer();
};

}  // namespace m1_lifecycle_demo

#endif  // M1_LIFECYCLE_DEMO__LIFECYCLE_NODE_HPP_
