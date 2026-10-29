# s03-g21-project2

MTRX3760 Project 2: Package Deal. ROS 2 workspace integrating adopted
wall follower, room shape and inventory packages.

## Layout
- `src/` : adopted packages (git subtree) and our own integration code

## Build (Ubuntu 24.04 + ROS 2 Jazzy)
```bash
sudo apt install ros-jazzy-desktop ros-jazzy-turtlebot3-gazebo ros-jazzy-ros-gz \
  ros-jazzy-cv-bridge ros-jazzy-generate-parameter-library libopencv-dev
source /opt/ros/jazzy/setup.bash
colcon build --symlink-install
source install/setup.bash
colcon test --packages-select mapper && colcon test-result --verbose
```

## Run in simulation
```bash
ros2 launch roomshape combined_rooms.launch.py          # gui:=false for headless
ros2 topic echo /mapper/status                          # run state and room count
```
The mapper waits for the wall follower's services, then switches it to the
left wall and starts it. When the run finishes (or on Ctrl+C mid-run) the map
is written to `mapper.dot` in the directory you launched from.

The arena (`src/inventory/src/worlds/combined_rooms.world`) is five rooms at
the classifier's sizes, joined as a tree by corridors with a tag on each
corridor floor; tile 34 is the start tile. Regenerate it after editing
`tools/make_combined_world.py`:
```bash
python3 tools/make_combined_world.py > src/inventory/src/worlds/combined_rooms.world
```
To test one doorway instead of the whole arena, spawn next to it, e.g.
`x_pose:=0.8 y_pose:=1.45` (inside the rectangle).

Known limitations: the wall follower can wedge on a corridor doorway, so a
full run does not always finish, and room labels can stay `unknown` because
roomshape also measures LiDAR rays that pass through doorways.

## Run on the TurtleBot3
Use the same `ROS_DOMAIN_ID` on the robot and the laptop.

On the robot (`ssh ubuntu@10.70.139.NN`, password `turtlebot`):
```bash
ros2 launch wall_follower hardware.launch.py use_camera:=true   # or: bringup, then cam
```
On the laptop:
```bash
ros2 launch roomshape combined_real_robot.launch.py start_tile_id:=<start tag>
```
Set `start_tile_id` to the ArUco ID on the start tile (also in
`src/mapper/config/mapper.yaml`). With it set, the run ends when the robot
sees the start tile again after crossing every passage both ways; left at -1,
it ends as soon as the last passage is crossed.
