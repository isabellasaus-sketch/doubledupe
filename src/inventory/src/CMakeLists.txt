################################################################################
# Set minimum required version of cmake, project name and compile options
################################################################################
cmake_minimum_required(VERSION 3.8)
project(inventory)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 20)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

################################################################################
# Find ament packages and libraries for ament and system dependencies
################################################################################
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(std_msgs REQUIRED)
find_package(cv_bridge REQUIRED)
find_package(OpenCV REQUIRED)

################################################################################
# Build
################################################################################
include_directories(
  include
  ${OpenCV_INCLUDE_DIRS}
)

# Executable 1: main inventory node
set(MAIN_EXEC_NAME "inventory_main")

add_executable(${MAIN_EXEC_NAME}
  src/CArucoReader.cpp
  src/CArucoProcessor.cpp
  src/CCamera.cpp
  src/CEncounter.cpp
  src/CTagState.cpp
  src/CInventory.cpp
  src/main.cpp)
target_link_libraries(${MAIN_EXEC_NAME}
  ${sensor_msgs_TARGETS}
  ${std_msgs_TARGETS}
  ${cv_bridge_TARGETS}
  rclcpp::rclcpp
  ${OpenCV_LIBS}
)

# Executable 2: webcam publisher node
set(WEBCAM_EXEC_NAME "webcam_publisher")

add_executable(${WEBCAM_EXEC_NAME}
  src/CWebcamPublisher.cpp
  src/webcam_main.cpp)
target_link_libraries(${WEBCAM_EXEC_NAME}
  ${sensor_msgs_TARGETS}
  ${std_msgs_TARGETS}
  ${cv_bridge_TARGETS}
  rclcpp::rclcpp
  ${OpenCV_LIBS}
)

# Executable 3: marker generator (its main() is in CTestMaker.cpp)
set(TEST_EXEC_NAME "inventory_test")

add_executable(${TEST_EXEC_NAME}
  src/CTestMaker.cpp
  src/CArucoReader.cpp)
target_link_libraries(${TEST_EXEC_NAME}
  ${sensor_msgs_TARGETS}
  ${std_msgs_TARGETS}
  ${cv_bridge_TARGETS}
  rclcpp::rclcpp
  ${OpenCV_LIBS}
)
# Executable 4: CEncounter unit test (pure C++, no ROS needed)
set(ENCOUNTER_TEST_EXEC_NAME "encounter_test")

add_executable(${ENCOUNTER_TEST_EXEC_NAME}
  src/CEncounter.cpp
  src/CTagState.cpp
  src/test.cpp)


################################################################################
# Install
################################################################################
install(TARGETS
  ${MAIN_EXEC_NAME}
  ${WEBCAM_EXEC_NAME}
  ${TEST_EXEC_NAME}
  ${ENCOUNTER_TEST_EXEC_NAME}
  DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY include/
  DESTINATION include/
)

install(DIRECTORY launch
  DESTINATION share/${PROJECT_NAME}/
)

# Gazebo world with ArUco floor tiles, and the tile models it includes.
install(DIRECTORY worlds models
  DESTINATION share/${PROJECT_NAME}/
)

# The integration test plays the bag from the installed share folder.
# Guarded because git does not track empty folders, so a fresh clone
# without bags/ would otherwise fail to configure.
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/bags)
  install(DIRECTORY bags
    DESTINATION share/${PROJECT_NAME}/
  )
endif()

################################################################################
# Testing
################################################################################
if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  find_package(ament_cmake_ros REQUIRED)
  find_package(launch_testing_ament_cmake REQUIRED)

  # Linters to skip (the unit has its own C++ style guidelines)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  set(ament_cmake_uncrustify_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()

  # Runs each test with its own ROS domain ID so tests on a shared lab
  # network don't interfere with each other (from the launch_testing tutorial)
  function(add_ros_isolated_launch_test path)
    set(RUNNER "${ament_cmake_ros_DIR}/run_test_isolated.py")
    add_launch_test("${path}" RUNNER "${RUNNER}" ${ARGN})
  endfunction()

  # Each test starts inventory.launch.py, plays one bag from bags/ and checks
  # the inventory the node ends with.
  # TIMEOUT (s) must be longer than the bag plus the delays in the test.
  # Lower it once you have re-recorded a short bag.
  add_ros_isolated_launch_test(test/webcam_test.py TIMEOUT 180)
  add_ros_isolated_launch_test(test/robot_duplicates1_test.py TIMEOUT 180)

  # Pure C++ unit test of the encounter logic, needs no ROS graph.
  add_test(NAME encounter_test COMMAND encounter_test)

endif()

################################################################################
# Macro for ament package
################################################################################
ament_export_include_directories(include)
ament_export_dependencies(
  rclcpp
  sensor_msgs
  std_msgs
  cv_bridge
)
ament_package()