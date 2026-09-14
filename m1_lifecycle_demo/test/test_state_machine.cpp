#include <gtest/gtest.h>

#include <memory>

#include <rclcpp/rclcpp.hpp>
#include <rclcpp_lifecycle/state.hpp>
#include <lifecycle_msgs/msg/state.hpp>
#include <std_msgs/msg/u_int64.hpp>

#include "m1_lifecycle_demo/lifecycle_node.hpp"

using m1_lifecycle_demo::LifecycleDemoNode;

class LifecycleFixture : public ::testing::Test
{
protected:
  static void SetUpTestSuite() {rclcpp::init(0, nullptr);}
  static void TearDownTestSuite() {rclcpp::shutdown();}
};

TEST_F(LifecycleFixture, configure_transitions_to_inactive) {
  auto node = std::make_shared<LifecycleDemoNode>(rclcpp::NodeOptions());
  const auto & state = node->configure();
  EXPECT_EQ(state.id(), lifecycle_msgs::msg::State::PRIMARY_STATE_INACTIVE);
}

TEST_F(LifecycleFixture, activate_transitions_to_active) {
  auto node = std::make_shared<LifecycleDemoNode>(rclcpp::NodeOptions());
  const auto & cfg_state = node->configure();
  const auto & act_state = node->activate();
  EXPECT_EQ(act_state.id(), lifecycle_msgs::msg::State::PRIMARY_STATE_ACTIVE);
}

TEST_F(LifecycleFixture, tick_increments_count) {
  auto node = std::make_shared<LifecycleDemoNode>(rclcpp::NodeOptions());
  const auto & cfg = node->configure();
  ASSERT_EQ(cfg.id(), lifecycle_msgs::msg::State::PRIMARY_STATE_INACTIVE);
  node->tick();
  node->tick();
  node->tick();
  EXPECT_EQ(node->get_tick_count(), 3u);
}

TEST_F(LifecycleFixture, cleanup_resets_tick_count) {
  auto node = std::make_shared<LifecycleDemoNode>(rclcpp::NodeOptions());
  const auto & cfg = node->configure();
  ASSERT_EQ(cfg.id(), lifecycle_msgs::msg::State::PRIMARY_STATE_INACTIVE);
  node->tick();
  node->tick();
  const auto & clean_state = node->cleanup();
  EXPECT_EQ(node->get_tick_count(), 0u);
}

TEST_F(LifecycleFixture, deactivate_transitions_to_inactive) {
  auto node = std::make_shared<LifecycleDemoNode>(rclcpp::NodeOptions());
  node->configure();
  const auto & act = node->activate();
  ASSERT_EQ(act.id(), lifecycle_msgs::msg::State::PRIMARY_STATE_ACTIVE);
  const auto & deact = node->deactivate();
  EXPECT_EQ(deact.id(), lifecycle_msgs::msg::State::PRIMARY_STATE_INACTIVE);
}

TEST_F(LifecycleFixture, cleanup_transitions_to_unconfigured) {
  auto node = std::make_shared<LifecycleDemoNode>(rclcpp::NodeOptions());
  node->configure();
  const auto & clean = node->cleanup();
  EXPECT_EQ(clean.id(), lifecycle_msgs::msg::State::PRIMARY_STATE_UNCONFIGURED);
}

TEST_F(LifecycleFixture, shutdown_transitions_to_finalized) {
  auto node = std::make_shared<LifecycleDemoNode>(rclcpp::NodeOptions());
  node->configure();
  const auto & sd = node->shutdown();
  EXPECT_EQ(sd.id(), lifecycle_msgs::msg::State::PRIMARY_STATE_FINALIZED);
}

TEST_F(LifecycleFixture, failed_configure_stays_unconfigured) {
  auto node = std::make_shared<LifecycleDemoNode>(rclcpp::NodeOptions());
  node->set_fail_on_configure(true);
  const auto & state = node->configure();
  // FAILURE 返回 → 状态不推进，回到源状态 UNCONFIGURED
  EXPECT_EQ(state.id(), lifecycle_msgs::msg::State::PRIMARY_STATE_UNCONFIGURED);
}
