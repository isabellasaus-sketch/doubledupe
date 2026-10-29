# main.launch.py
# Single launch file for simulation or hardware mode.
#
# Usage:
#   Simulation (laptop):
#     ros2 launch wall_follower launch.py sim_time:=true
#     ros2 launch wall_follower launch.py sim_time:=false

import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.conditions import IfCondition, UnlessCondition
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node

PACKAGE_NAME = 'wall_follower'
PACKAGE_PATH = get_package_share_directory(PACKAGE_NAME)
LAUNCH_DIR = os.path.join(PACKAGE_PATH, 'launch')


def generate_launch_description():
    # Read variables
    declare_use_sim_time = DeclareLaunchArgument('sim_time', default_value='true')

    declare_usb_port = DeclareLaunchArgument(
        'usb_port', default_value='/dev/ttyACM0',
        description='USB port connected to OpenCR')
    declare_x_pose = DeclareLaunchArgument('x_pose', default_value='1.0')
    declare_y_pose = DeclareLaunchArgument('y_pose', default_value='-2.0')
    declare_z_pose = DeclareLaunchArgument('z_pose', default_value='0.1')

    usb_port = LaunchConfiguration('usb_port')
    x_pose = LaunchConfiguration('x_pose')
    y_pose = LaunchConfiguration('y_pose')
    z_pose = LaunchConfiguration('z_pose')

    use_sim_time = LaunchConfiguration('sim_time')

    sim_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(os.path.join(LAUNCH_DIR, 'sim.launch.py')),
        launch_arguments={'x_pose': x_pose,
                          'y_pose': y_pose,
                          'z_pose': z_pose}.items(),
        condition=IfCondition(use_sim_time))

    hardware_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(os.path.join(LAUNCH_DIR, 'hardware.launch.py')),
        launch_arguments={'usb_port': usb_port}.items(),
        condition=UnlessCondition(use_sim_time))

    wall_follower_params = None
    config_file = os.path.join(PACKAGE_PATH, 'config', 'wall_follower_config.yaml')
    if not os.path.isfile(config_file):
        print('Cannot find wall_follower_config.yaml file inside wall_follower/config')
        return LaunchDescription([])

    wall_follower_params = config_file

    wall_follower_node = Node(
        package=PACKAGE_NAME,
        executable='wall_follower_drive',
        output='screen',
        parameters=[
            wall_follower_params,
            {'use_sim_time': use_sim_time}
        ]
    )

    return LaunchDescription([
        declare_use_sim_time,
        declare_usb_port,
        declare_x_pose,
        declare_y_pose,
        declare_z_pose,
        sim_launch,
        hardware_launch,
        wall_follower_node
    ])
