# core.launch.py
# Launches the rviz and visualisation tools
# ros2 launch wall_follower base_station.launch.py

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch_ros.actions import Node
from launch.conditions import IfCondition
import os

PACKAGE_NAME="wall_follower"
PACKAGE_PATH = get_package_share_directory(PACKAGE_NAME)
RVIZ_CONFIG_DIR = os.path.join(PACKAGE_PATH, 'config')

def generate_launch_description():
    declare_use_sim_time = DeclareLaunchArgument('use_sim_time', default_value='true')
    declare_rviz = DeclareLaunchArgument('rviz', default_value='true')
    declare_rviz_file = DeclareLaunchArgument('rviz_file', default_value='rviz_config.rviz')

    use_sim_time = LaunchConfiguration('use_sim_time')
    use_rviz = LaunchConfiguration('rviz')
    rviz_file = LaunchConfiguration('rviz_file')

    rviz_node = Node(
        package='rviz2',
        executable='rviz2',
        output='screen',
        arguments=['-d', PathJoinSubstitution([RVIZ_CONFIG_DIR, rviz_file])],
        parameters=[{'use_sim_time': use_sim_time}],
        condition=IfCondition(use_rviz))

    path_recorder_node = Node(
        package=PACKAGE_NAME,
        executable='path_recorder',
        output='screen',
        parameters=[{'use_sim_time' : use_sim_time}]
    )

    return LaunchDescription([
        declare_use_sim_time,
        declare_rviz,
        declare_rviz_file,
        rviz_node,
        path_recorder_node
    ])
