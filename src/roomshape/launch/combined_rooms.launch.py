"""Launch the connected multi-room world with live marker detection."""

import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import AppendEnvironmentVariable, DeclareLaunchArgument
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():
    inventory_share = get_package_share_directory('inventory')
    roomshape_share = get_package_share_directory('roomshape')
    wall_follower_share = get_package_share_directory('wall_follower')
    mapper_share = get_package_share_directory('mapper')

    # The camera-equipped Burger is needed for live floor-tag detection.
    os.environ['TURTLEBOT3_MODEL'] = 'burger_cam'

    world_path = os.path.join(
        inventory_share, 'worlds', 'combined_rooms.world')
    roomshape_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(os.path.join(
            roomshape_share, 'launch', 'gazebo.launch.py')),
        launch_arguments={
            'world': world_path,
            'gui': LaunchConfiguration('gui'),
            'x_pose': LaunchConfiguration('x_pose'),
            'y_pose': LaunchConfiguration('y_pose'),
        }.items())

    camera_bridge = Node(
        package='ros_gz_bridge',
        executable='parameter_bridge',
        arguments=[
            '/camera/image_raw@sensor_msgs/msg/Image[gz.msgs.Image',
            '/camera/camera_info@sensor_msgs/msg/CameraInfo[gz.msgs.CameraInfo',
        ],
        output='screen',
    )

    wall_follower = Node(
        package='wall_follower',
        executable='wall_follower_drive',
        name='wall_follower_drive_node',
        output='screen',
        parameters=[
            os.path.join(
                wall_follower_share, 'config', 'wall_follower_config.yaml'),
            # The mapper starts the follower once every node is up.
            {'use_sim_time': True, 'start_running': False},
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
        remappings=[('camera/image_raw', '/camera/image_raw')],
    )

    mapper = Node(
        package='mapper',
        executable='mapper_node',
        output='screen',
        parameters=[
            os.path.join(mapper_share, 'config', 'mapper.yaml'),
            # Tile 34 is the start tile in combined_rooms.world.
            {'use_sim_time': True, 'start_tile_id': 34},
        ],
    )

    return LaunchDescription([
        DeclareLaunchArgument(
            'gui',
            default_value='true',
            choices=['true', 'false'],
            description='Show the Gazebo GUI'),
        # The default is the start tile in the small square. Spawning elsewhere
        # is useful for testing one doorway without driving the whole arena.
        DeclareLaunchArgument('x_pose', default_value='-0.7'),
        DeclareLaunchArgument('y_pose', default_value='0.45'),
        AppendEnvironmentVariable(
            'GZ_SIM_RESOURCE_PATH',
            os.path.join(inventory_share, 'models')),
        roomshape_launch,
        camera_bridge,
        wall_follower,
        inventory,
        mapper,
    ])
