cmake_minimum_required(VERSION 3.8)
project(mapper)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(std_srvs REQUIRED)
find_package(wall_follower REQUIRED)

# Optional: keep the map model as a library if it is part of the mapper design
add_library(map_model
  src/Passage.cpp
  src/Room.cpp
  src/TopologyGraph.cpp
)

target_compile_features(map_model PUBLIC cxx_std_17)

target_include_directories(map_model PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

add_executable(mapper_node
  src/main.cpp
  src/MapperNode.cpp
  src/RunStateMachine.cpp
  src/WallFollowerClient.cpp
)

target_compile_features(mapper_node PUBLIC cxx_std_17)

ament_target_dependencies(mapper_node
  rclcpp
  std_msgs
  std_srvs
  wall_follower
)

target_link_libraries(mapper_node map_model)

target_include_directories(mapper_node PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

install(
  TARGETS mapper_node map_model
  DESTINATION lib/${PROJECT_NAME}
)

install(
  DIRECTORY include/
  DESTINATION include
)

# Lets launch files load the mapper parameters from the package share folder.
install(
  DIRECTORY config
  DESTINATION share/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  find_package(ament_cmake_gtest REQUIRED)

  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  set(ament_cmake_uncrustify_FOUND TRUE)

  ament_lint_auto_find_test_dependencies()

  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test/test_map_model.cpp")
    ament_add_gtest(test_map_model test/test_map_model.cpp)
    target_link_libraries(test_map_model map_model)
  endif()

  ament_add_gtest(test_RunStateMachine
    test/test_RunStateMachine.cpp
    src/RunStateMachine.cpp
  )
  target_include_directories(test_RunStateMachine PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
  )

  # Runs against a fake follower in a private namespace, so no robot is needed.
  ament_add_gtest(test_WallFollowerClient
    test/test_WallFollowerClient.cpp
    src/WallFollowerClient.cpp
    TIMEOUT 60
  )
  target_include_directories(test_WallFollowerClient PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
  )
  ament_target_dependencies(test_WallFollowerClient
    rclcpp
    std_srvs
    wall_follower
  )
endif()

ament_package()