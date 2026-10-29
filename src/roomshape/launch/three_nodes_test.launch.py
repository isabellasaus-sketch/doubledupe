# Starts Room Shape and Wall Follower in Gazebo while Inventory processes a
# prerecorded camera bag, so all three adopted nodes run concurrently.

import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.actions import ExecuteProcess
from launch.actions import IncludeLaunchDescription
from launch.actions import TimerAction
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():
    roomshape_share = get_package_share_directory('roomshape')
    inventory_share = get_package_share_directory('inventory')
    wall_follower_share = get_package_share_directory('wall_follower')

    roomshape_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(os.path.join(
            roomshape_share, 'launch', 'gazebo.launch.py')),
        launch_arguments={
            'world': LaunchConfiguration('world'),
            'gui': LaunchConfiguration('gui'),
        }.items())

    wall_follower_node = Node(
        package='wall_follower',
        executable='wall_follower_drive',
        name='wall_follower_drive_node',
        output='screen',
        parameters=[
            os.path.join(
                wall_follower_share, 'config', 'wall_follower_config.yaml'),
            {'use_sim_time': True},
        ])

    inventory_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(os.path.join(
            inventory_share, 'launch', 'inventory.launch.py')))

    camera_bag = os.path.join(
        inventory_share, 'bags', 'webcam_test')
    bag_player = TimerAction(
        period=5.0,
        actions=[ExecuteProcess(
            cmd=['ros2', 'bag', 'play', camera_bag],
            output='screen',
        )])

    return LaunchDescription([
        DeclareLaunchArgument(
            'world',
            default_value='small_square.sdf',
            description='Room Shape Gazebo world to run'),
        DeclareLaunchArgument(
            'gui',
            default_value='true',
            choices=['true', 'false'],
            description='Show the Gazebo GUI'),
        roomshape_launch,
        wall_follower_node,
        inventory_launch,
        bag_player,
    ])
