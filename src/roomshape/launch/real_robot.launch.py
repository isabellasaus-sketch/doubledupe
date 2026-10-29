# MTRX3760 2026 Project 2: Package Deal
# File: real_robot.launch.py
# Starts only the classifier for a running robot driver or an external scan source.

# ---Imports-------------------------------------------------------------------
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.actions import OpaqueFunction
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue


# No Gazebo lookup, robot spawning or hardware bringup is performed here.
def _launch_setup(context):
    use_sim_time = LaunchConfiguration('use_sim_time').perform(context) == 'true'
    return [Node(
        package='roomshape',
        executable='roomshape_node',
        name='roomshape_node',
        output='screen',
        parameters=[{
            'use_sim_time': use_sim_time,
            'match_tolerance': ParameterValue(
                LaunchConfiguration('match_tolerance'), value_type=float),
            'required_stable_scans': ParameterValue(
                LaunchConfiguration('required_stable_scans'), value_type=int),
            'minimum_valid_points': ParameterValue(
                LaunchConfiguration('minimum_valid_points'), value_type=int),
        }],
        remappings=[
            ('scan', LaunchConfiguration('scan_topic')),
            ('room_shape/type', LaunchConfiguration('result_topic')),
        ],
    )]


def generate_launch_description():
    return LaunchDescription([
        DeclareLaunchArgument(
            'use_sim_time', default_value='false', choices=['true', 'false'],
            description='Use true only when connecting to an external simulator'),
        DeclareLaunchArgument(
            'scan_topic', default_value='/scan',
            description='LaserScan input topic'),
        DeclareLaunchArgument(
            'result_topic', default_value='/room_shape/type',
            description='Detected room type output topic'),
        DeclareLaunchArgument(
            'match_tolerance', default_value='0.18',
            description='Maximum error for each measured side in metres'),
        DeclareLaunchArgument(
            'required_stable_scans', default_value='3',
            description='Consecutive matching scans required before output'),
        DeclareLaunchArgument(
            'minimum_valid_points', default_value='80',
            description='Minimum valid LiDAR points required for an estimate'),
        OpaqueFunction(function=_launch_setup),
    ])
