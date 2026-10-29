# MTRX3760 2026 Project 2: Package Deal
# File: test_roomshape_launch.py
# Checks real node output using the recorded small-square LaserScan bag.

# ---Imports-------------------------------------------------------------------
import os
import time
import unittest

from ament_index_python.packages import get_package_share_directory
import launch
from launch.actions import ExecuteProcess
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
import launch_testing
import launch_testing.actions
import rclpy
from rclpy.qos import DurabilityPolicy
from rclpy.qos import QoSProfile
from rclpy.qos import ReliabilityPolicy
from std_msgs.msg import String


# ---generate_test_description-------------------------------------------------
# Exercise scan conversion, classification and stability without Gazebo.
def generate_test_description():
    roomshape_share = get_package_share_directory('roomshape')
    bag_path = os.path.join(
        roomshape_share,
        'bags',
        'small_square_demo')

    # Exercise the robot/node-only entry point using bag data, not hardware.
    roomshape_node = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(os.path.join(
            roomshape_share, 'launch', 'real_robot.launch.py')))

    # Delay playback briefly to give the node time to discover the scan topic.
    bag_player = ExecuteProcess(
        cmd=[
            'ros2', 'bag', 'play', bag_path,
            '--delay', '1.0',
        ],
        output='screen')

    return (
        launch.LaunchDescription([
            roomshape_node,
            bag_player,
            launch_testing.actions.ReadyToTest(),
        ]),
        {
            'roomshape_node': roomshape_node,
            'bag_player': bag_player,
        },
    )


# ---TestRoomShapeBagOutput-----------------------------------------------------
# Own the test-side ROS context and release its node after each assertion.
class TestRoomShapeBagOutput(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        rclpy.init()

    @classmethod
    def tearDownClass(cls):
        rclpy.shutdown()

    def setUp(self):
        self.node = rclpy.create_node('roomshape_integration_test_listener')

    def tearDown(self):
        self.node.destroy_node()

    def test_small_square_is_reported(self):
        received_room_types = []
        # Match the publisher's durability so a late listener can receive the
        # retained result even when the node suppresses duplicate labels.
        output_qos = QoSProfile(
            depth=1,
            reliability=ReliabilityPolicy.RELIABLE,
            durability=DurabilityPolicy.TRANSIENT_LOCAL)

        subscription = self.node.create_subscription(
            String,
            '/room_shape/type',
            lambda message: received_room_types.append(message.data),
            output_qos)

        try:
            # Use wall time: bag playback need not provide a simulation clock.
            deadline = time.monotonic() + 15.0
            while time.monotonic() < deadline:
                rclpy.spin_once(self.node, timeout_sec=0.25)
                if 'SMALL_SQUARE' in received_room_types:
                    break

            self.assertIn(
                'SMALL_SQUARE',
                received_room_types,
                'The demo bag did not produce the expected room type.')
        finally:
            self.node.destroy_subscription(subscription)


# ---TestRoomShapeShutdown------------------------------------------------------
# Check exit codes after launch_testing has stopped the launched processes.
@launch_testing.post_shutdown_test()
class TestRoomShapeShutdown(unittest.TestCase):

    def test_processes_exit_cleanly(self, proc_info):
        launch_testing.asserts.assertExitCodes(proc_info)
