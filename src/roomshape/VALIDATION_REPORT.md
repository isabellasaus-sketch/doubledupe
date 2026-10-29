# Validation report

## Static validator retired

At the team's request, `scripts/validate_package.py` was emptied while keeping
the file. It no longer performs checks. Static-validator results below are
historical observations from before this change. The optional Gazebo script
and the required bag integration test are unchanged.

## Optional-test removal (20 October 2026)

The classifier unit test and both packages' launch-configuration tests were
removed at the team's request, together with their CMake registrations.
The Room Shape bag integration test and Wall Follower hand-made-scan integration
test remain. The static validator no longer requires the removed classifier
test. Historical results below describe earlier revisions, not the current
test count or current automatic coverage.

Verification after removal:

- Both packages built successfully in a fresh external build/install directory.
- Both retained isolated integration tests passed: Room Shape reported
  `SMALL_SQUARE` from the bag; Wall Follower passed start/follow/turn/stop checks.
- CTest registered six targets for Room Shape (integration plus lint checks)
  and one for Wall Follower. None of the three removed targets remain.
- Combined `colcon test-result`: 31 tests, 0 errors, 0 failures, 11 skipped.
  The skips are cppcheck checks affected by the installed 2.13.0 version.
- Static package checks passed, including the seven remaining C++ file headers.
- No new Gazebo or physical-robot run was performed for this removal.

## Historical two-entry launch verification (before optional-test removal)

- Each package now has `gazebo.launch.py` and `real_robot.launch.py`.
- Both packages built successfully in a fresh external build/install directory.
- Combined `colcon test-result`: 36 tests, 0 errors, 0 failures, 12 skipped.
  The skips are cppcheck checks affected by the installed tool version.
- Both isolated integration tests passed: hand-made scans for Wall Follower
  and the included bag for Room Shape.
- Eight Room Shape and seven Wall Follower launch-configuration cases passed.
- Both installed launch entries exposed the expected arguments.
- Headless Gazebo smoke checks: Room Shape received scans and reported
  `SMALL_SQUARE`; Wall Follower received scans and reported `STOPPED`.
  Both launch processes shut down after SIGINT. No real robot was used and
  the wall follower was not enabled for motion during these smoke checks.
- The existing five-world and standard-world driving records were not repeated
  as part of this change; continuous wall-following/classification needs its
  own validation.

## Earlier unified launch verification (20 October 2026)

- Clean ROS 2 Jazzy build: PASS.
- CTest: 9/9 test targets passed; cppcheck remains skipped internally due to
  the installed version's known performance issues.
- Eight launch-configuration cases: PASS, including node-only defaults,
  world paths, clock overrides, topic/parameter forwarding and spawn arguments.
- Rosbag integration through the public launch with no world: PASS,
  output `SMALL_SQUARE`, and clean process shutdown.
- Installed `ros2 launch roomshape roomshape.launch.py --show-args`: PASS.
- No new live Gazebo run was performed for this launch consolidation; the
  five-world observations below belong to the earlier verification.

## Historical package checks (before optional-test removal)

- Package metadata and required-file check: PASS
- Exact five-world and two-launch-file set check: PASS
- Python syntax for both launch files: PASS
- XML parsing for all five SDF world files: PASS
- Computed internal dimensions for all four rectangular worlds: PASS
- Computed 1.00 m internal side lengths for the equilateral triangle: PASS
- C++17 compilation of the independent classifier test: PASS
- Twelve classifier cases, including triangle and noisy measurements: PASS
- Bash syntax for the automatic Gazebo test script: PASS
- Scan-geometry simulation for all five room definitions: PASS
- Mandatory Project 2 headers on all eight C++ files: PASS
- ROS 2 Jazzy-format demo bag containing 40 `/scan` messages (approximately 8.0 s): PASS
- Isolated `launch_testing` test source and CMake registration: PASS

## World matrix

| World | Internal size | Expected output |
| --- | --- | --- |
| `small_square` | 1.00 m x 1.00 m | `SMALL_SQUARE` |
| `large_square` | 1.50 m x 1.50 m | `LARGE_SQUARE` |
| `rectangle` | 0.50 m x 1.00 m | `RECTANGLE` |
| `hallway` | 0.45 m x 1.45 m | `HALLWAY` |
| `equilateral_triangle` | 1.00 m equal sides | `EQUILATERAL_TRIANGLE` |

## Earlier target-computer verification (before launch consolidation)

The package includes `scripts/test_all_worlds.sh` for the final ROS 2 Jazzy,
TurtleBot3 and Gazebo Sim end-to-end test. Run it from the workspace root after building:

```bash
bash src/roomshape/scripts/test_all_worlds.sh "$(pwd)"
```

The automated five-world test was executed successfully, with the following observed result:

```text
Result: 5/5 Gazebo worlds passed.
```

All five worlds were individually verified in Gazebo Sim on Ubuntu with ROS 2 Jazzy, including the equilateral triangle. Each produced its expected stable classification.

The package also includes `test/test_roomshape_launch.py`. It starts the
RoomShape node, plays `bags/small_square_demo`, listens to
`/room_shape/type`, and requires the output `SMALL_SQUARE`. The test is
registered with the isolated ROS test runner and was verified on the
target ROS 2 Jazzy computer with `colcon test`.

### Automated test results

- CTest: 8/8 tests passed.
- Test summary: 38 tests, 0 errors, 0 failures, 8 skipped.
- Rosbag integration test: PASS, output SMALL_SQUARE.
- Test processes exited cleanly.
- cppcheck 2.13.0 was skipped due to known performance issues.
