# MTRX3760 2026 Project 2: Package Deal
# File: inventory.launch.py
# Author(s): Joshua Kwok, Davin
#
# Starts the inventory node. The camera topic and the two encounter
# parameters can be overridden from the command line or from another
# launch file, e.g.
#   ros2 launch inventory inventory.launch.py image_topic:=/my_camera/image
#   ros2 launch inventory inventory.launch.py absence_timeout_s:=2.0

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue


def generate_launch_description():
    image_topic = LaunchConfiguration('image_topic')
    absence_timeout_s = LaunchConfiguration('absence_timeout_s')
    min_consecutive_frames = LaunchConfiguration('min_consecutive_frames')

    return LaunchDescription([
        DeclareLaunchArgument(
            'image_topic',
            default_value='/camera/image_raw',
            description='Camera image topic the node subscribes to'),
        DeclareLaunchArgument(
            'absence_timeout_s',
            default_value='1.0',
            description='Seconds a tag must be out of view before it can be a new encounter'),
        DeclareLaunchArgument(
            'min_consecutive_frames',
            default_value='1',
            description='Frames in a row a tag must be seen to count as an encounter'),

        Node(
            package='inventory',
            executable='inventory_main',
            output='screen',
            # Launch arguments are strings, so give the parameters their real types.
            parameters=[{
                'absence_timeout_s': ParameterValue(absence_timeout_s, value_type=float),
                'min_consecutive_frames': ParameterValue(min_consecutive_frames, value_type=int),
            }],
            remappings=[('camera/image_raw', image_topic)],
        ),
    ])
