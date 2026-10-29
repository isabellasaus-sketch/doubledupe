"""
Run the full mapping stack on a laptop against a real TurtleBot3.

The robot itself only runs its drivers and camera, e.g.
    ros2 launch wall_follower hardware.launch.py use_camera:=true
(or the lab `bringup` and `cam` aliases). Everything below runs on the laptop:
    ros2 launch roomshape combined_real_robot.launch.py start_tile_id:=<id>
"""

import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue


def generate_launch_description():
    roomshape_share = get_package_share_directory('roomshape')
    wall_follower_share = get_package_share_directory('wall_follower')
    mapper_share = get_package_share_directory('mapper')

    # The scan and camera come from the robot, so no node uses sim time.
    roomshape = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(os.path.join(
            roomshape_share, 'launch', 'real_robot.launch.py')),
        launch_arguments={'use_sim_time': 'false'}.items())

    wall_follower = Node(
        package='wall_follower',
        executable='wall_follower_drive',
        name='wall_follower_drive_node',
        output='screen',
        parameters=[
            os.path.join(
                wall_follower_share, 'config', 'wall_follower_config.yaml'),
            # The mapper starts the follower once every node is up.
            {'use_sim_time': False, 'start_running': False},
        ],
    )

    inventory = Node(
        package='inventory',
        executable='inventory_main',
        output='screen',
        parameters=[{
            'absence_timeout_s': 1.0,
            'min_consecutive_frames': 1,
        }],
        remappings=[('camera/image_raw', LaunchConfiguration('image_topic'))],
    )

    mapper = Node(
        package='mapper',
        executable='mapper_node',
        output='screen',
        parameters=[
            os.path.join(mapper_share, 'config', 'mapper.yaml'),
            {
                'use_sim_time': False,
                'start_tile_id': ParameterValue(
                    LaunchConfiguration('start_tile_id'), value_type=int),
                'output_path': LaunchConfiguration('output_path'),
            },
        ],
    )

    return LaunchDescription([
        DeclareLaunchArgument(
            'start_tile_id', default_value='-1',
            description='ArUco ID on the start tile (-1: finish as soon as '
                        'every passage is crossed both ways)'),
        DeclareLaunchArgument(
            'image_topic', default_value='/camera/image_raw',
            description='Camera image topic published by the robot'),
        DeclareLaunchArgument(
            'output_path', default_value='mapper.dot',
            description='Where the finished map is written'),
        roomshape,
        wall_follower,
        inventory,
        mapper,
    ])
