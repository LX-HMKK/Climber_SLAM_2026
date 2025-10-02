cmake_minimum_required(VERSION 3.14)
project(rm_communication)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_action REQUIRED)  # 新增此行
find_package(std_msgs REQUIRED)
find_package(message_filters REQUIRED)
find_package(serial REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(nav2_msgs REQUIRED)
find_package(rosidl_default_generators REQUIRED)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

rosidl_generate_interfaces(${PROJECT_NAME}
  "action/NavigateToPose.action"
  DEPENDENCIES nav2_msgs
)


ament_package()

add_executable(sub_from_nav src/sub_from_nav.cpp)
ament_target_dependencies(sub_from_nav rclcpp)

install(TARGETS
  sub_from_nav
  DESTINATION lib/${PROJECT_NAME}
)


add_executable(talker src/talker.cpp)
ament_target_dependencies(talker rclcpp std_msgs geometry_msgs serial nav2_msgs rclcpp_action)

add_executable(lis src/lis.cpp)
ament_target_dependencies(lis rclcpp message_filters serial std_msgs)

install(TARGETS
  talker
  lis
  DESTINATION lib/${PROJECT_NAME}
)


add_executable(monitor src/monitor.cpp)
ament_target_dependencies(monitor rclcpp message_filters serial std_msgs)

install(TARGETS
  monitor
  DESTINATION lib/${PROJECT_NAME}
)
