"""Integration tests for the wall_follower ROS 2 package."""

# Tests the real wall_follower_drive node without Gazebo or a robot.

import math
import time
import unittest

from ament_index_python.packages import get_package_share_directory
from geometry_msgs.msg import TwistStamped
import launch
import launch.actions
import launch_ros.actions
import launch_testing
import launch_testing.actions
import rclpy
from rclpy.qos import qos_profile_sensor_data
from sensor_msgs.msg import LaserScan
from std_srvs.srv import SetBool, Trigger
from wall_follower.msg import WallFollowerState


def generate_test_description():
    """Launch the wall follower with its configuration."""
    package_share = get_package_share_directory('wall_follower')

    # make wall follower node
    wall_follower_node = launch_ros.actions.Node(
        package='wall_follower',
        executable='wall_follower_drive',
        name='wall_follower_drive_node',
        output='screen',
        parameters=[
            package_share + '/config/wall_follower_config.yaml'
        ],
    )

    # delays tests to give time for nodes to see each other
    return (
        launch.LaunchDescription([
            wall_follower_node,
            launch.actions.TimerAction(
                period=1.0,
                actions=[
                    launch_testing.actions.ReadyToTest()
                ],
            ),
        ]),
        {},
    )


# Test Class
class TestWallFollower(unittest.TestCase):
    """Integration tests for LaserScan, TwistStamped, and services."""

    @classmethod
    def setUpClass(cls):
        """Initialise the ROS 2 client library."""
        rclpy.init()

    @classmethod
    def tearDownClass(cls):
        """Shut down the ROS 2 client library."""
        rclpy.shutdown()

    # creates publishers and subscribers
    def setUp(self):
        """Create a test node and its ROS interfaces."""
        # creates python ros node for the test
        self.node = rclpy.create_node('wall_follower_integration_test')
        self.commands = []
        self.state_msgs = []

        # Match the best-effort QoS used by the wall follower's scan
        # subscription (rclcpp::SensorDataQoS()).
        self.scan_pub = self.node.create_publisher(
            LaserScan,
            '/scan',
            qos_profile_sensor_data,
        )

        self.cmd_sub = self.node.create_subscription(
            TwistStamped,
            '/cmd_vel',
            self.cmd_vel_callback,
            10,
        )

        self.state_sub = self.node.create_subscription(
            WallFollowerState,
            '/wall_follower/state',
            self.state_callback,
            10,
        )

        self.start_cli = self.node.create_client(
            Trigger,
            '/wall_follower/start',
        )
        self.stop_cli = self.node.create_client(
            Trigger,
            '/wall_follower/stop',
        )
        self.set_running_cli = self.node.create_client(
            SetBool,
            '/wall_follower/set_running',
        )
        self.follow_left_cli = self.node.create_client(
            SetBool,
            '/wall_follower/follow_left',
        )

    def tearDown(self):
        """Reset node state and destroy the test node."""
        try:
            self.call_service(
                self.start_cli, Trigger.Request(), timeout_sec=1.0
            )
            req = SetBool.Request()
            req.data = False
            self.call_service(self.follow_left_cli, req, timeout_sec=1.0)
        except Exception:
            pass
        self.node.destroy_node()

    def cmd_vel_callback(self, msg):
        """Store velocity commands received from the wall follower."""
        self.commands.append(msg)

    def state_callback(self, msg):
        """Store state messages received from the wall follower."""
        self.state_msgs.append(msg)

    def call_service(self, client, request, timeout_sec=5.0):
        """Call a ROS service and wait for the response."""
        self.assertTrue(
            client.wait_for_service(timeout_sec=timeout_sec),
            f'Service {client.srv_name} not available',
        )
        future = client.call_async(request)
        deadline = time.monotonic() + timeout_sec
        while time.monotonic() < deadline:
            rclpy.spin_once(self.node, timeout_sec=0.05)
            if future.done():
                return future.result()
        self.fail(f'Service {client.srv_name} timed out after {timeout_sec}s')

    # makes the artificial laser scan data
    def make_scan(
        self,
        front_distance,
        front_right_distance,
        right_distance,
    ):
        """
        Create a synthetic 360-degree laser scan.

        The specified distances are placed in the sectors consumed by
        LidarProcessor:
          0 degrees:   front
          -45 degrees: front-right
          -90 degrees: right

        All other rays return 2.0 metres.
        """
        scan = LaserScan()

        scan.header.stamp = self.node.get_clock().now().to_msg()
        scan.header.frame_id = 'base_scan'

        # angle range with 500 lidar rays
        scan.angle_min = -math.pi
        scan.angle_max = math.pi
        scan.angle_increment = (
            scan.angle_max - scan.angle_min
        ) / 500.0

        scan.time_increment = 0.0
        scan.scan_time = 0.1
        scan.range_min = 0.12
        scan.range_max = 3.5

        # 501 rays spanning -180 to +180 degrees.
        scan.ranges = [2.0] * 501

        # Set each ray according to its angle. Use independent conditions
        # so every ray in the specified angular window gets the right range.
        for index in range(len(scan.ranges)):
            angle = (
                scan.angle_min
                + index * scan.angle_increment
            )

            if abs(angle) <= math.radians(5.0):
                scan.ranges[index] = front_distance

            if abs(angle + math.radians(45.0)) <= math.radians(5.0):
                scan.ranges[index] = front_right_distance

            if abs(angle + math.radians(90.0)) <= math.radians(5.0):
                scan.ranges[index] = right_distance

        return scan

    def wait_for_connections(self, timeout_sec=5.0):
        """Wait until the test and wall follower discover each other."""
        deadline = time.monotonic() + timeout_sec

        while time.monotonic() < deadline:
            rclpy.spin_once(self.node, timeout_sec=0.05)

            scan_connected = (
                self.scan_pub.get_subscription_count() > 0
            )
            command_connected = (
                self.cmd_sub.get_publisher_count() > 0
            )

            if scan_connected and command_connected:
                return

        self.fail(
            'ROS connections were not established: '
            'the wall follower must subscribe to /scan and '
            'publish /cmd_vel.'
        )

    # connects the sensor simulation to the controller and gets response
    def publish_scans_until_command(
        self,
        front_distance,
        front_right_distance,
        right_distance,
    ):
        """
        Publish repeated scans and wait for a fresh velocity command.

        Repeated publishing allows for DDS discovery and ensures the node
        receives the synthetic scan without requiring a robot or simulator.
        """
        self.wait_for_connections()

        # Discard commands received before this test scenario.
        self.commands.clear()

        scan = self.make_scan(
            front_distance,
            front_right_distance,
            right_distance,
        )

        # publish repeatedly
        publish_deadline = time.monotonic() + 0.5

        while time.monotonic() < publish_deadline:
            scan.header.stamp = self.node.get_clock().now().to_msg()
            self.scan_pub.publish(scan)

            # Process incoming commands while publishing scans.
            rclpy.spin_once(self.node, timeout_sec=0.02)

        # The controller timer runs every 10 ms. Continue processing until
        # we receive a command produced after the scan publishing period.
        response_deadline = time.monotonic() + 3.0

        while time.monotonic() < response_deadline:
            rclpy.spin_once(self.node, timeout_sec=0.05)

            if self.commands:
                return self.commands[-1]

        self.fail(
            'No TwistStamped command received on /cmd_vel '
            'after publishing synthetic LaserScan messages.'
        )

    # check the result
    def assert_command(self, command, expected_linear, expected_angular):
        """Check the linear and angular velocities against expected values."""
        self.assertIsInstance(command, TwistStamped)

        self.assertTrue(
            math.isfinite(command.twist.linear.x),
            'Linear velocity must be finite.',
        )

        self.assertTrue(
            math.isfinite(command.twist.angular.z),
            'Angular velocity must be finite.',
        )

        self.assertAlmostEqual(
            command.twist.linear.x,
            expected_linear,
            delta=0.001,
            msg='Unexpected linear velocity.',
        )

        self.assertAlmostEqual(
            command.twist.angular.z,
            expected_angular,
            delta=0.001,
            msg='Unexpected angular velocity.',
        )

    # TESTS

    def test_wall_follower_tracks_wall(self):
        """Ideal wall distances should produce straight movement."""
        command = self.publish_scans_until_command(
            front_distance=2.0,
            front_right_distance=0.3535533905932738,
            right_distance=0.2,
        )

        self.assert_command(
            command,
            expected_linear=0.10,
            expected_angular=0.0,
        )

    def test_front_obstacle(self):
        """A close front obstacle should stop forward motion and turn."""
        command = self.publish_scans_until_command(
            front_distance=0.20,
            front_right_distance=0.3535533905932738,
            right_distance=0.2,
        )

        self.assert_command(
            command,
            expected_linear=0.0,
            expected_angular=0.2,
        )

    def test_wall_too_close(self):
        """A nearby right wall should produce a left-turn command."""
        command = self.publish_scans_until_command(
            front_distance=2.0,
            front_right_distance=0.2,
            right_distance=0.10,
        )

        self.assert_command(
            command,
            expected_linear=0.05,
            expected_angular=0.2,
        )

    def test_wall_too_far(self):
        """A distant wall should produce a right-turn command."""
        command = self.publish_scans_until_command(
            front_distance=2.0,
            front_right_distance=0.60,
            right_distance=0.50,
        )

        self.assert_command(
            command,
            expected_linear=0.05,
            expected_angular=-0.2,
        )

    def test_runtime_service_stop_and_start(self):
        """Runtime services should stop and start wall following."""
        self.wait_for_connections()

        # Stop the wall follower
        response = self.call_service(self.stop_cli, Trigger.Request())
        self.assertTrue(response.success, 'Stop service call failed')

        # Wait until state reflects PAUSED
        deadline = time.monotonic() + 2.0
        while time.monotonic() < deadline:
            rclpy.spin_once(self.node, timeout_sec=0.05)
            if self.state_msgs and (
                self.state_msgs[-1].state == WallFollowerState.PAUSED
            ):
                break

        self.assertTrue(len(self.state_msgs) > 0)
        self.assertEqual(self.state_msgs[-1].state, WallFollowerState.PAUSED)

        # Clear commands and spin to capture commands produced during PAUSED
        self.commands.clear()
        spin_deadline = time.monotonic() + 0.2
        while time.monotonic() < spin_deadline:
            rclpy.spin_once(self.node, timeout_sec=0.05)

        self.assertTrue(
            len(self.commands) > 0,
            'No command published while stopped',
        )
        self.assertAlmostEqual(
            self.commands[-1].twist.linear.x, 0.0, delta=0.001
        )
        self.assertAlmostEqual(
            self.commands[-1].twist.angular.z, 0.0, delta=0.001
        )

        # Restart the wall follower
        response = self.call_service(self.start_cli, Trigger.Request())
        self.assertTrue(response.success, 'Start service call failed')

        # Publish scans and verify motion resumes
        command = self.publish_scans_until_command(
            front_distance=2.0,
            front_right_distance=0.3535533905932738,
            right_distance=0.2,
        )
        self.assert_command(
            command,
            expected_linear=0.10,
            expected_angular=0.0,
        )
        self.assertEqual(
            self.state_msgs[-1].state, WallFollowerState.FOLLOWING
        )

    def test_runtime_service_set_running(self):
        """The set_running service should pause and unpause the node."""
        self.wait_for_connections()

        # Pause node via set_running(False)
        req = SetBool.Request()
        req.data = False
        response = self.call_service(self.set_running_cli, req)
        self.assertTrue(response.success)

        # Wait until state reflects PAUSED
        deadline = time.monotonic() + 2.0
        while time.monotonic() < deadline:
            rclpy.spin_once(self.node, timeout_sec=0.05)
            if self.state_msgs and (
                self.state_msgs[-1].state == WallFollowerState.PAUSED
            ):
                break

        self.assertEqual(self.state_msgs[-1].state, WallFollowerState.PAUSED)

        self.commands.clear()
        spin_deadline = time.monotonic() + 0.2
        while time.monotonic() < spin_deadline:
            rclpy.spin_once(self.node, timeout_sec=0.05)

        self.assertTrue(len(self.commands) > 0)
        self.assertAlmostEqual(
            self.commands[-1].twist.linear.x, 0.0, delta=0.001
        )

        # Unpause node via set_running(True)
        req.data = True
        response = self.call_service(self.set_running_cli, req)
        self.assertTrue(response.success)

        # Drive command should resume
        command = self.publish_scans_until_command(
            front_distance=2.0,
            front_right_distance=0.3535533905932738,
            right_distance=0.2,
        )
        self.assert_command(
            command,
            expected_linear=0.10,
            expected_angular=0.0,
        )
        self.assertEqual(
            self.state_msgs[-1].state, WallFollowerState.FOLLOWING
        )

    def test_runtime_service_follow_left(self):
        """The follow_left service should toggle the followed wall side."""
        self.wait_for_connections()

        # Switch to left wall
        req = SetBool.Request()
        req.data = True
        response = self.call_service(self.follow_left_cli, req)
        self.assertTrue(response.success)

        # Wait for state message confirming follow_left
        deadline = time.monotonic() + 2.0
        while time.monotonic() < deadline:
            rclpy.spin_once(self.node, timeout_sec=0.05)
            if self.state_msgs and self.state_msgs[-1].follow_left is True:
                break

        self.assertTrue(self.state_msgs[-1].follow_left)

        # Reset back to right wall
        req.data = False
        response = self.call_service(self.follow_left_cli, req)
        self.assertTrue(response.success)

        deadline = time.monotonic() + 2.0
        while time.monotonic() < deadline:
            rclpy.spin_once(self.node, timeout_sec=0.05)
            if self.state_msgs and self.state_msgs[-1].follow_left is False:
                break

        self.assertFalse(self.state_msgs[-1].follow_left)
