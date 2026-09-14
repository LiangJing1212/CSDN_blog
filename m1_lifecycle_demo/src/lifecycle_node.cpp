#include "m1_lifecycle_demo/lifecycle_node.hpp"

#include <chrono>
#include <string>

#include <rclcpp/rclcpp.hpp>
#include <rclcpp_lifecycle/lifecycle_node.hpp>

namespace m1_lifecycle_demo
{

using namespace std::chrono_literals;

LifecycleDemoNode::LifecycleDemoNode(const rclcpp::NodeOptions & options)
: rclcpp_lifecycle::LifecycleNode("lifecycle_demo", options)
{
  RCLCPP_INFO(get_logger(), "LifecycleDemoNode constructed");
}

rclcpp_lifecycle::State LifecycleDemoNode::get_state()
{
  return get_current_state();
}

void LifecycleDemoNode::tick() {++tick_count_;}

void LifecycleDemoNode::on_timer()
{
  if (tick_count_ > 0) {
    std_msgs::msg::UInt64 msg;
    msg.data = static_cast<uint64_t>(tick_count_);
    tick_pub_->publish(msg);
  }
}

LifecycleDemoNode::CallbackReturn LifecycleDemoNode::on_configure(
  const rclcpp_lifecycle::State & state)
{
  RCLCPP_INFO(get_logger(), "on_configure() from %s", state.label().c_str());

  if (fail_on_configure_) {
    RCLCPP_WARN(get_logger(), "configure FAILURE injected");
    return CallbackReturn::FAILURE;
  }

  tick_count_ = 0;
  tick_pub_ = create_publisher<std_msgs::msg::UInt64>("lifecycle_tick", 10);

  timer_ = create_wall_timer(200ms, [this]() {on_timer();});

  return CallbackReturn::SUCCESS;
}

LifecycleDemoNode::CallbackReturn LifecycleDemoNode::on_activate(
  const rclcpp_lifecycle::State & state)
{
  RCLCPP_INFO(get_logger(), "on_activate() from %s", state.label().c_str());

  tick_pub_->on_activate();
  timer_ = create_wall_timer(200ms, [this]() {on_timer();});
  return CallbackReturn::SUCCESS;
}

LifecycleDemoNode::CallbackReturn LifecycleDemoNode::on_deactivate(
  const rclcpp_lifecycle::State & state)
{
  RCLCPP_INFO(get_logger(), "on_deactivate() from %s", state.label().c_str());

  timer_.reset();
  tick_pub_->on_deactivate();
  return CallbackReturn::SUCCESS;
}

LifecycleDemoNode::CallbackReturn LifecycleDemoNode::on_cleanup(
  const rclcpp_lifecycle::State & state)
{
  RCLCPP_INFO(get_logger(), "on_cleanup() from %s", state.label().c_str());

  timer_.reset();
  tick_pub_.reset();
  tick_count_ = 0;
  return CallbackReturn::SUCCESS;
}

LifecycleDemoNode::CallbackReturn LifecycleDemoNode::on_shutdown(
  const rclcpp_lifecycle::State & state)
{
  RCLCPP_INFO(get_logger(), "on_shutdown() from %s", state.label().c_str());

  if (timer_) {timer_.reset();}
  if (tick_pub_) {tick_pub_.reset();}
  return CallbackReturn::SUCCESS;
}

}  // namespace m1_lifecycle_demo
