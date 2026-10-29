# MTRX3760 2026 Project 2: Package Deal
# File: CMakeLists.txt
# Build the ROS node and register the integration test and style checks.

# ---Build configuration------------------------------------------------------
cmake_minimum_required(VERSION 3.20)
project(roomshape)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# ---Dependencies-------------------------------------------------------------
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(std_msgs REQUIRED)

# ---Room Shape library-------------------------------------------------------
# Scan processing and classification, kept free of the node entry point.
add_library(roomshape_lib
  src/CRoomClassifier.cpp
  src/CRoomScanProcessor.cpp
)

target_compile_features(roomshape_lib PUBLIC cxx_std_17)

target_include_directories(roomshape_lib PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

ament_target_dependencies(roomshape_lib
  rclcpp
  sensor_msgs
  std_msgs
)

# ---Room Shape executable----------------------------------------------------
add_executable(roomshape_node
  src/CRoomShapeNode.cpp
  src/main.cpp
)

target_compile_features(roomshape_node PUBLIC cxx_std_17)

target_include_directories(roomshape_node PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

ament_target_dependencies(roomshape_node
  rclcpp
  sensor_msgs
  std_msgs
)

target_link_libraries(roomshape_node roomshape_lib)

# ---Installation-------------------------------------------------------------
# ros2 run expects package executables under lib/<package>.
install(
  TARGETS roomshape_node roomshape_lib
  DESTINATION lib/${PROJECT_NAME}
)

install(
  DIRECTORY include/
  DESTINATION include
)

# Package-share resources are found through the ament index at runtime.
install(
  DIRECTORY bags launch worlds
  DESTINATION share/${PROJECT_NAME}
)

# ---Tests--------------------------------------------------------------------
if(BUILD_TESTING)
  enable_testing()
  # Run launch tests with a unique ROS_DOMAIN_ID so tests on a shared
  # laboratory network cannot communicate with one another.
  find_package(ament_cmake_ros REQUIRED)
  find_package(launch_testing_ament_cmake REQUIRED)

  function(add_ros_isolated_launch_test path)
    set(RUNNER "${ament_cmake_ros_DIR}/run_test_isolated.py")
    add_launch_test("${path}" RUNNER "${RUNNER}" ${ARGN})
  endfunction()

  add_ros_isolated_launch_test(
    test/test_roomshape_launch.py
    TIMEOUT 30
  )

  find_package(ament_lint_auto REQUIRED)
  # Keep the project header and C++ style conventions without formatter checks.
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  set(ament_cmake_uncrustify_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()

  # Check authored Python only, not stale symlinks in an in-package install/.
  list(APPEND AMENT_LINT_AUTO_EXCLUDE
    ament_cmake_flake8
    ament_cmake_pep257
  )
  ament_flake8(launch scripts test)
  ament_pep257(launch scripts test)
endif()

ament_package()
