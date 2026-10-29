# hardware.launch.py
# Launches robot.launch.py (state publisher, lidar, turtlebot3_node) and optionally the camera.
# TURTLEBOT3_MODEL must be 'burger' (not burger_cam) and LDS_MODEL must be 'LDS-04' (LDROBOT LD19);
# both are set here rather than exported.
# Run this ON the Turtlebot.
#
# Example Usage:
# ros2 launch wall_follower hardware.launch.py
# ros2 launch wall_follower hardware.launch.py use_camera:=false
# ros2 launch wall_follower hardware.launch.py usb_port:=/dev/ttyACM1
# ros2 launch wall_follower hardware.launch.py usb_port:=/dev/ttyACM1 use_camera:=false

import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription, SetEnvironmentVariable
from launch.conditions import IfCondition
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration

TURTLEBOT3_MODEL = 'burger'
LDS_MODEL = 'LDS-04'

BRINGUP_DIR = os.path.join(get_package_share_directory('turtlebot3_bringup'), 'launch')


def generate_launch_description():
    declare_use_camera = DeclareLaunchArgument(
        'use_camera', default_value='false',
        description='Launch the camera (set false if the camera is faulty)')
    declare_namespace = DeclareLaunchArgument('namespace', default_value='')

    usb_port = LaunchConfiguration('usb_port')
    use_camera = LaunchConfiguration('use_camera')
    namespace = LaunchConfiguration('namespace')

    # These run before the includes below, so robot.launch.py sees them when it reads os.environ
    set_model_env = SetEnvironmentVariable('TURTLEBOT3_MODEL', TURTLEBOT3_MODEL)
    set_lds_env = SetEnvironmentVariable('LDS_MODEL', LDS_MODEL)

    robot_cmd = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(BRINGUP_DIR, 'robot.launch.py')),
        launch_arguments={
            'usb_port': usb_port,
            'namespace': namespace,
        }.items())

    camera_cmd = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(BRINGUP_DIR, 'camera.launch.py')),
        launch_arguments={
            'camera': '0',
            'format': '',
            'use_image_view': 'false',
            'width': '640',
            'height': '480',
        }.items(),
        condition=IfCondition(use_camera))

    return LaunchDescription([
        declare_use_camera,
        declare_namespace,
        set_model_env,
        set_lds_env,
        robot_cmd,
        camera_cmd,
    ])