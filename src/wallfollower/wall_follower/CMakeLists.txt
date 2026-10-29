cmake_minimum_required(VERSION 3.8)
project(wall_follower)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(std_msgs REQUIRED)
find_package(std_srvs REQUIRED)
find_package(tf2 REQUIRED)
find_package(generate_parameter_library REQUIRED)
find_package(rosidl_default_generators REQUIRED)

include_directories(
  include
)

set(msg_files
  "msg/WallFollowerState.msg"
)

rosidl_generate_interfaces(${PROJECT_NAME}
  ${msg_files}
  DEPENDENCIES std_msgs
)

set(EXEC_NAME "wall_follower_drive")

generate_parameter_library(
  wall_follower_params
  src/wall_follower_params.yaml
)

add_executable(${EXEC_NAME}
  src/wall_follower_drive.cpp
  src/wall_follower_controller.cpp
  src/lidar_processor.cpp
)

rosidl_get_typesupport_target(cpp_typesupport_target "${PROJECT_NAME}" "rosidl_typesupport_cpp")

target_link_libraries(${EXEC_NAME}
  PRIVATE
  wall_follower_params
  ${cpp_typesupport_target}
  ${geometry_msgs_TARGETS}
  ${nav_msgs_TARGETS}
  ${sensor_msgs_TARGETS}
  ${std_msgs_TARGETS}
  ${std_srvs_TARGETS}
  rclcpp::rclcpp
  tf2::tf2
)

add_executable(path_recorder src/path_recorder.cpp)

target_link_libraries(path_recorder
  PRIVATE
  ${geometry_msgs_TARGETS}
  ${nav_msgs_TARGETS}
  rclcpp::rclcpp
)

install(TARGETS ${EXEC_NAME} path_recorder
  DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY launch config worlds
  DESTINATION share/${PROJECT_NAME}/
)

install(DIRECTORY include/
  DESTINATION include/
)

ament_export_include_directories(include)
ament_export_dependencies(
  geometry_msgs
  nav_msgs
  rclcpp
  sensor_msgs
  tf2
  std_msgs
  std_srvs
  rosidl_default_runtime
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()

  # Integration test dependencies and isolated test runner.
  find_package(ament_cmake_ros REQUIRED)
  find_package(launch_testing_ament_cmake REQUIRED)

  function(add_ros_isolated_launch_test path)
    set(RUNNER "${ament_cmake_ros_DIR}/run_test_isolated.py")
    add_launch_test(
      "${path}"
      RUNNER "${RUNNER}"
      ${ARGN}
    )
  endfunction()

  add_ros_isolated_launch_test(
    test/test_wall_follower.py
    TIMEOUT 60
  )
endif()

ament_package()
