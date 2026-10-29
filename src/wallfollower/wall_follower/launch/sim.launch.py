# sim.launch.py
# Launch file for simulation environment: Gazebo TurtleBot3 world, robot spawn, robot_state_publisher

# """Simulation: Gazebo TurtleBot3 world, robot spawn, robot_state_publisher."""
import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import AppendEnvironmentVariable
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription, SetEnvironmentVariable
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration

PACKAGE_NAME="wall_follower"
PACKAGE_PATH = get_package_share_directory(PACKAGE_NAME)
TB3_GAZEBO = get_package_share_directory('turtlebot3_gazebo')
ROS_GZ_SIM = get_package_share_directory('ros_gz_sim')
TB3_LAUNCH_DIR = os.path.join(TB3_GAZEBO, 'launch')
TURTLEBOT3_MODEL = 'burger_cam'

def generate_launch_description():
    declare_use_sim_time = DeclareLaunchArgument('use_sim_time', default_value='true')
    declare_x_pose = DeclareLaunchArgument('x_pose', default_value='1.0')
    declare_y_pose = DeclareLaunchArgument('y_pose', default_value='-2.0')
    declare_z_pose = DeclareLaunchArgument('z_pose', default_value='0.1')

    use_sim_time = LaunchConfiguration('use_sim_time')
    x_pose = LaunchConfiguration('x_pose')
    y_pose = LaunchConfiguration('y_pose')
    z_pose = LaunchConfiguration('z_pose')

    set_model_env = SetEnvironmentVariable('TURTLEBOT3_MODEL', TURTLEBOT3_MODEL)

    world = os.path.join(TB3_GAZEBO, 'worlds', 'turtlebot3_world.world')

    gzserver_cmd = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(ROS_GZ_SIM, 'launch', 'gz_sim.launch.py')
        ),
        launch_arguments={'gz_args': ['-r -s -v2 ', world], 'on_exit_shutdown': 'true'}.items()
    )

    gzclient_cmd = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(ROS_GZ_SIM, 'launch', 'gz_sim.launch.py')
        ),
        launch_arguments={'gz_args': '-g -v2 ', 'on_exit_shutdown': 'true'}.items()
    )

    robot_state_publisher_cmd = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(TB3_LAUNCH_DIR, 'robot_state_publisher.launch.py')
        ),
        launch_arguments={'use_sim_time': use_sim_time}.items()
    )

    spawn_turtlebot_cmd = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(TB3_LAUNCH_DIR, 'spawn_turtlebot3.launch.py')
        ),
        launch_arguments={
            'x_pose': x_pose,
            'y_pose': y_pose,
            'z_pose': z_pose
        }.items()
    )

    set_env_vars_resources = AppendEnvironmentVariable(
            'GZ_SIM_RESOURCE_PATH',
            os.path.join(
                get_package_share_directory('turtlebot3_gazebo'),
                'models'))

    rviz_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(PACKAGE_PATH, 'launch', 'rviz.launch.py')),
        launch_arguments={'use_sim_time': use_sim_time}.items())

    return LaunchDescription([
        # Set the environment variales
        declare_use_sim_time,
        declare_x_pose,
        declare_y_pose,
        declare_z_pose,
        set_model_env,
        gzserver_cmd,
        gzclient_cmd,
        set_env_vars_resources,
        spawn_turtlebot_cmd,
        robot_state_publisher_cmd,
        rviz_launch,
    ])
 
    
