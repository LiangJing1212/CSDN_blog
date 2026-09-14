cmake_minimum_required(VERSION 3.16)
project(m1_lifecycle_demo LANGUAGES CXX)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_lifecycle REQUIRED)
find_package(lifecycle_msgs REQUIRED)
find_package(std_msgs REQUIRED)

add_library(lifecycle_node SHARED
  src/lifecycle_node.cpp
)
target_include_directories(lifecycle_node PUBLIC include)
ament_target_dependencies(lifecycle_node rclcpp rclcpp_lifecycle std_msgs)

add_executable(lifecycle_demo src/main.cpp)
target_link_libraries(lifecycle_demo lifecycle_node)
ament_target_dependencies(lifecycle_demo rclcpp rclcpp_lifecycle)

install(TARGETS lifecycle_node
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION lib/${PROJECT_NAME}
)

install(TARGETS lifecycle_demo
  RUNTIME DESTINATION lib/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()

  find_package(ament_cmake_gtest REQUIRED)
  ament_add_gtest(test_state_machine test/test_state_machine.cpp)
  target_link_libraries(test_state_machine lifecycle_node)
  ament_target_dependencies(test_state_machine rclcpp rclcpp_lifecycle lifecycle_msgs)
endif()

ament_package()
