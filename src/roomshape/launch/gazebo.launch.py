# MTRX3760 2026 Project 2: Package Deal
# File: gazebo.launch.py
# Starts Gazebo, TurtleBot3 and the room classifier.

# ---Imports-------------------------------------------------------------------
import os
import shlex

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import AppendEnvironmentVariable
from launch.actions import DeclareLaunchArgument
from launch.actions import IncludeLaunchDescription
from launch.actions import OpaqueFunction
from launch.conditions import IfCondition
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration


# ---_simulation_actions-------------------------------------------------------
# Look up simulator resources only when a world has actually been supplied.
def _simulation_actions(context, world_path, use_sim_time):
    # Upstream TurtleBot3 launch files read this environment variable.
    os.environ.setdefault('TURTLEBOT3_MODEL', 'burger')
    turtlebot3_share = get_package_share_directory('turtlebot3_gazebo')
    ros_gz_share = get_package_share_directory('ros_gz_sim')
    gui_enabled = IfCondition(LaunchConfiguration('gui')).evaluate(context)
    gazebo_args = '-r -v4 '
    if not gui_enabled:
        gazebo_args += '-s --headless-rendering '

    # Append rather than replace existing Gazebo model search directories.
    turtlebot3_models = AppendEnvironmentVariable(
        'GZ_SIM_RESOURCE_PATH',
        os.path.join(turtlebot3_share, 'models'))

    # Run the selected world immediately; stop the launch when Gazebo exits.
    gazebo = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(ros_gz_share, 'launch', 'gz_sim.launch.py')),
        launch_arguments={
            'gz_args': gazebo_args + shlex.quote(world_path),
            'on_exit_shutdown': 'true'
        }.items())

    # Publish robot transforms against the same simulation clock as the node.
    robot_state_publisher = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(
                turtlebot3_share,
                'launch',
                'robot_state_publisher.launch.py')),
        launch_arguments={
            'use_sim_time': str(use_sim_time).lower(),
        }.items())

    # The world geometry leaves the origin clear for the initial robot pose.
    spawn_turtlebot3 = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(
                turtlebot3_share,
                'launch',
                'spawn_turtlebot3.launch.py')),
        launch_arguments={
            'x_pose': LaunchConfiguration('x_pose'),
            'y_pose': LaunchConfiguration('y_pose'),
        }.items())

    return [
        turtlebot3_models,
        gazebo,
        robot_state_publisher,
        spawn_turtlebot3,
    ]


# ---_launch_setup-------------------------------------------------------------
def _launch_setup(context):
    # This entry point always starts a simulator; an empty world is an error.
    world = LaunchConfiguration('world').perform(context).strip()
    if not world:
        raise RuntimeError(
            'world must not be empty; use real_robot.launch.py for node-only mode')
    use_sim_time = LaunchConfiguration('use_sim_time').perform(context) == 'true'
    package_share = get_package_share_directory('roomshape')
    world_path = os.path.expanduser(world)
    if not os.path.isabs(world_path):
        world_path = os.path.join(package_share, 'worlds', world_path)
    if not os.path.isfile(world_path):
        raise RuntimeError(f'Gazebo world file does not exist: {world_path}')

    actions = _simulation_actions(context, world_path, use_sim_time)
    # Reuse the node-only entry point so both modes configure the same node.
    actions.append(IncludeLaunchDescription(
        PythonLaunchDescriptionSource(os.path.join(
            package_share, 'launch', 'real_robot.launch.py')),
        launch_arguments={
            'use_sim_time': str(use_sim_time).lower(),
            'scan_topic': LaunchConfiguration('scan_topic'),
            'result_topic': LaunchConfiguration('result_topic'),
            'match_tolerance': LaunchConfiguration('match_tolerance'),
            'required_stable_scans': LaunchConfiguration('required_stable_scans'),
            'minimum_valid_points': LaunchConfiguration('minimum_valid_points'),
        }.items(),
    ))
    return actions


def generate_launch_description():
    return LaunchDescription([
        DeclareLaunchArgument(
            'world', default_value='small_square.sdf',
            description='Gazebo world path. '
                        'Relative paths are under roomshape/worlds'),
        DeclareLaunchArgument(
            'use_sim_time', default_value='true',
            choices=['true', 'false'],
            description='Use the Gazebo simulation clock'),
        DeclareLaunchArgument(
            'gui', default_value='true', choices=['true', 'false'],
            description='Show Gazebo GUI when a world is supplied'),
        DeclareLaunchArgument(
            'x_pose', default_value='0.0',
            description='Robot start X coordinate in metres'),
        DeclareLaunchArgument(
            'y_pose', default_value='0.0',
            description='Robot start Y coordinate in metres'),
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
