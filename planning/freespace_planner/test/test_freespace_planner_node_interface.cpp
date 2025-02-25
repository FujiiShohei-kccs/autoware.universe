// Copyright 2023 TIER IV, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "ament_index_cpp/get_package_share_directory.hpp"
#include "freespace_planner/freespace_planner_node.hpp"
#include "planning_interface_test_manager/planning_interface_test_manager.hpp"
#include "planning_interface_test_manager/planning_interface_test_manager_utils.hpp"

#include <gtest/gtest.h>

#include <vector>

using freespace_planner::FreespacePlannerNode;
using planning_test_utils::PlanningInterfaceTestManager;

std::shared_ptr<PlanningInterfaceTestManager> generateTestManager()
{
  auto test_manager = std::make_shared<PlanningInterfaceTestManager>();
  test_manager->setRouteInputTopicName("freespace_planner/input/route");
  test_manager->setTrajectorySubscriber("freespace_planner/output/trajectory");
  test_manager->setOdometryTopicName("freespace_planner/input/odometry");
  test_manager->setInitialPoseTopicName("freespace_planner/input/odometry");
  return test_manager;
}

std::shared_ptr<FreespacePlannerNode> generateNode()
{
  auto node_options = rclcpp::NodeOptions{};
  const auto planning_test_utils_dir =
    ament_index_cpp::get_package_share_directory("planning_test_utils");
  const auto freespace_planner_dir =
    ament_index_cpp::get_package_share_directory("freespace_planner");
  node_options.arguments(
    {"--ros-args", "--params-file",
     planning_test_utils_dir + "/config/test_vehicle_info.param.yaml", "--params-file",
     freespace_planner_dir + "/config/freespace_planner.param.yaml"});
  return std::make_shared<FreespacePlannerNode>(node_options);
}

void publishMandatoryTopics(
  std::shared_ptr<PlanningInterfaceTestManager> test_manager,
  rclcpp::Node::SharedPtr test_target_node)
{
  // publish necessary topics from test_manager
  test_manager->publishTF(test_target_node, "/tf");
  test_manager->publishOdometry(test_target_node, "freespace_planner/input/odometry");
  test_manager->publishOccupancyGrid(test_target_node, "freespace_planner/input/occupancy_grid");
  test_manager->publishParkingScenario(test_target_node, "freespace_planner/input/scenario");
}

TEST(PlanningModuleInterfaceTest, testPlanningInterfaceWithVariousTrajectoryInput)
{
  rclcpp::init(0, nullptr);

  auto test_manager = generateTestManager();
  auto test_target_node = generateNode();
  publishMandatoryTopics(test_manager, test_target_node);

  // test with normal route
  ASSERT_NO_THROW(test_manager->testWithBehaviorNominalRoute(test_target_node));
  EXPECT_GE(test_manager->getReceivedTopicNum(), 1);

  // test with empty route
  ASSERT_NO_THROW(test_manager->testWithAbnormalRoute(test_target_node));
  rclcpp::shutdown();
}

TEST(PlanningModuleInterfaceTest, NodeTestWithOffTrackEgoPose)
{
  rclcpp::init(0, nullptr);
  auto test_manager = generateTestManager();
  auto test_target_node = generateNode();

  publishMandatoryTopics(test_manager, test_target_node);

  // test for normal route
  ASSERT_NO_THROW(test_manager->testWithBehaviorNominalRoute(test_target_node));
  EXPECT_GE(test_manager->getReceivedTopicNum(), 1);

  ASSERT_NO_THROW(test_manager->testRouteWithInvalidEgoPose(test_target_node));

  rclcpp::shutdown();
}
